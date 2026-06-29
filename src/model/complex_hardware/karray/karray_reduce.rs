use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_dynamic_index::{mux_into_wire, pack_scalar_karray};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::model_arena::ModelArena;

// ---- generic callback-driven Karray reduce ----------------------------------
//
// Generalises the dynamic-index mux tree (`karray_dynamic_index.rs`): instead of a
// fixed index-encoded select, the caller decides the select-left at each 2:1 node.
// The tree is built the same balanced way; only the select signal comes from the
// caller (in the Python binding, that is a user callback per compared pair). This
// is the Rust port of the C++ `Table::doReduceBase(..., cusLogic, ...)`. The reduce
// LOOP itself is driven by the connector (it must release the arena borrow around
// the Python callback); this file only provides the leaf / mux / pack primitives.

/// How a dimension participates in a reduce: pinned to one index, or folded
/// (fanned out and reduced over its full extent).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ReduceDim {
    Pin (usize),
    Fold,
}

/// One subtree of the reduction: the element coordinates it covers (`indices`),
/// and the current per-field signal (`fields`, parallel name+HCP). At a leaf the
/// HCP is the element's own field HCP; after a mux it is a fresh mux-output wire.
/// All fields are `Copy`/owned, so a node holds no arena borrow and survives across
/// the connector's Python callback.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct ReduceNode {
    // The element coordinates this subtree covers: outer vec = the set of covered
    // elements, inner vec = one element's per-dimension coordinate. A leaf holds one
    // coordinate; a mux concatenates its two children's coverage. E.g. reducing a 1-D
    // array of 4: leaves [[0]] [[1]] [[2]] [[3]] -> [[0],[1]] [[2],[3]] -> the winner
    // [[0],[1],[2],[3]]. For a 2-D (2,3) array a coordinate is [row, col], e.g. [1,2].
    pub indices : Vec<Vec<usize>>,
    pub fields  : Vec<(String, HcpIdent)>,
}

impl Karray {
    /// Fan out the `Fold` dimensions (pinning the `Pin` ones) into one leaf
    /// `ReduceNode` per selected element, in row-major order. Each leaf reads its
    /// selected fields' own HCPs. Reg/Wire backings only (mirrors the dynamic read).
    pub fn reduce_leaves(&self, dim_sels: &[ReduceDim], selected_fields: &[usize], arena: &mut ModelArena) -> Vec<ReduceNode> {

        // integrity check
        assert!(matches!(self.get_backing(), HwComponentType::Reg | HwComponentType::Wire),
            "reduce_leaves: only Reg- or Wire-backed Karrays support reduce, got {:?}", self.get_backing());
        assert_eq!(dim_sels.len(), self.get_dim_size(),
            "reduce_leaves: expected {} dim selectors (one per dimension), got {}", self.get_dim_size(), dim_sels.len());
        for (d, sel) in dim_sels.iter().enumerate() {
            if let ReduceDim::Pin(i) = sel {
                assert!(*i < self.get_shape()[d],
                    "reduce_leaves: pinned index {i} out of bounds for dim {d} of size {}", self.get_shape()[d]);
            }
        }

        let mut out       = Vec::new();
        let mut cur_coord = Vec::with_capacity(self.get_dim_size());
        self.collect_leaves(0, &mut cur_coord, dim_sels, selected_fields, arena, &mut out);
        out
    }

    // Recursive fan-out: Pin fixes the dim, Fold loops its full extent; at a fully
    // pinned coordinate read each selected field's HCP into a leaf node.
    fn collect_leaves(&self, dim_idx        : usize,
                             cur_coord      : &mut Vec<usize>,
                             dim_sels       : &[ReduceDim],
                             selected_fields: &[usize],
                             arena          : &mut ModelArena,
                             out            : &mut Vec<ReduceNode> ) {
        if dim_idx == self.get_dim_size() {
            // the dead-end case
            let mut fields = Vec::with_capacity(selected_fields.len());
            for &field_idx in selected_fields {
                let hcp = self.static_index_get_hcp(cur_coord, field_idx, true, arena);
                fields.push((self.get_fields()[field_idx].get_name().to_string(), hcp));
            }
            out.push(ReduceNode { indices: vec![cur_coord.clone()], fields });
            return;
        }

        match dim_sels[dim_idx] {
            ReduceDim::Pin(i) => {
                cur_coord.push(i);
                self.collect_leaves(dim_idx + 1, cur_coord, dim_sels, selected_fields, arena, out);
                cur_coord.pop();
            }
            ReduceDim::Fold => {
                for i in 0..self.get_shape()[dim_idx] {
                    cur_coord.push(i);
                    self.collect_leaves(dim_idx + 1, cur_coord, dim_sels, selected_fields, arena, out);
                    cur_coord.pop();
                }
            }
        }
    }

    /// Mux two subtrees field-by-field under `select_left` (true picks `a`), each
    /// into a fresh wire. The merged node covers both subtrees' indices.
    pub fn reduce_mux(&self, a: &ReduceNode, b: &ReduceNode, select_left: HcpIdent, arena: &mut ModelArena) -> ReduceNode {
        assert_eq!(a.fields.len(), b.fields.len(), "reduce_mux: field count mismatch");
        let mut fields = Vec::with_capacity(a.fields.len());
        for (f, (name, a_hcp)) in a.fields.iter().enumerate() {
            let b_hcp = b.fields[f].1;
            let width = arena.get_hw_bit_sz(a_hcp);
            let res_w = arena.make_wire(false, &format!("{}_RMUX", a_hcp.get_global_name()), width);
            mux_into_wire(arena, res_w, *a_hcp, b_hcp, select_left);
            fields.push((name.clone(), res_w));
        }
        let mut indices = a.indices.clone();
        indices.extend(b.indices.iter().cloned());
        ReduceNode { indices, fields }
    }

    /// Pack the winning subtree's fields into a fresh wire-backed scalar Karray
    /// (shape `[1]`), returning its CcpIdent (the readable winner element).
    pub fn reduce_finish(&self, winner: &ReduceNode, arena: &mut ModelArena) -> CcpIdent {
        let fields: Vec<(String, i32)> = winner.fields.iter()
            .map(|(name, hcp)| (name.clone(), arena.get_hw_bit_sz(hcp)))
            .collect();
        let src: Vec<HcpIdent> = winner.fields.iter().map(|(_, hcp)| *hcp).collect();
        let name = format!("{}_REDUCE", self.get_ccp_ident().get_global_name());
        pack_scalar_karray(&name, fields, &src, arena)
    }
}
