use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_dynamic_get::{mux_into_wire, pack_scalar_karray};
use crate::model::complex_hardware::karray::karray_region_sel::KarrayAsmErr;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::model_arena::ModelArena;

// ===== Generic callback-driven Karray reduce — types + arena building blocks ==
//
// Reduce a Karray's elements to a single winner by a user-supplied select rule —
// the Rust port of the C++ `Table::doReduceBase(..., cusLogic, ...)`. It generalises
// the dynamic-index mux tree (`karray_dynamic_get.rs`): instead of an index-encoded
// select, the CALLER decides the select-left at each 2:1 node.
//
// This file holds the reduce TYPES and the arena building blocks (`reduce_prepare` /
// `reduce_leaf`, `reduce_mux`, `reduce_pack`). The `ReduceEnv` trait and the recursive
// `reduce_run` algorithm that drives them live in `karray_dynamic_reduce_get_run.rs`.

// ---- types ------------------------------------------------------------------

/// A named signal carried through the reduce — a karray field or a user extra.
pub type NamedHcp = (String, HcpIdent);

/// Per-dimension reduce plan: pin to one index, or fold (reduce over its extent).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ReduceDim {
    Pin (usize),
    Fold,
}

// (The `ReduceEnv` trait + the `reduce_run` algorithm live in `karray_dynamic_reduce_get_run.rs`.)

// ---- arena building blocks (the connector's ReduceEnv impl calls these) ------

impl Karray {
    /// Validate a reduce request and resolve it: checks the backing (Reg/Wire), the
    /// per-dimension selector count and pin bounds, then resolves the field names to
    /// indices. Returns `(shape, field_idxs)` for the recursive driver to fan over; an
    /// unknown field name is a `Value` error.
    ///
    pub fn reduce_prepare(&self, field_names: &[String], dim_sels: &[ReduceDim])
        -> Result<(Vec<usize>, Vec<usize>), KarrayAsmErr> {
               ///   ^-------------^----- shape of karray
               ///                 |---------- field idx
        // step1: backing must be Reg or Wire (MemBlock is addressed, not muxed).
        assert!(matches!(self.get_backing(), HwComponentType::Reg | HwComponentType::Wire),
            "reduce: only Reg- or Wire-backed Karrays support reduce, got {:?}", self.get_backing());

        // step2: one selector per dimension.
        assert_eq!(dim_sels.len(), self.get_dim_size(),
            "reduce: expected {} dim selectors (one per dimension), got {}", self.get_dim_size(), dim_sels.len());

        // step3: every pinned index must be within its dimension.
        for (d, sel) in dim_sels.iter().enumerate() {
            if let ReduceDim::Pin(i) = sel {
                assert!(*i < self.get_shape()[d],
                    "reduce: pinned index {i} out of bounds for dim {d} of size {}", self.get_shape()[d]);
            }
        }

        // step4: resolve field names to indices (any miss -> a clean Value error).
        let mut field_idxs = Vec::with_capacity(field_names.len());
        let mut missing    = Vec::new();
        for name in field_names {
            match self.field_index(name) {
                Some(field_idx) => field_idxs.push(field_idx),
                None            => missing.push(name.clone()),
            }
        }
        if !missing.is_empty() {
            return Err(KarrayAsmErr::Value(format!("reduce: no such field(s): {missing:?}")));
        }

        // step5: hand back the shape (to fan over) and the resolved field indices.
        Ok((self.get_shape().clone(), field_idxs))
    }

    /// Read ONE element's fields at the fully-pinned `coord` (parallel to `field_idxs`).
    /// The recursive driver calls this at each leaf — no flat pool of all elements.
    pub fn reduce_leaf(&self, coord: &[usize], selected_field_idxs: &[usize], arena: &mut ModelArena) -> Vec<NamedHcp> {
        selected_field_idxs.iter().map(|&fi|
            ( // named hcp
             self.get_fields()[fi].get_name().to_string(),
             self.static_index_get_hcp(coord, fi, true, arena)
            )
        ).collect()
    }
}

/// Mux `(name, a_hcp, b_hcp)` triples under `sel` (true picks `a`), each into a fresh
/// wire; returns the merged `(name, wire)` list in input order.
pub fn reduce_mux(arena: &mut ModelArena, pairs: &[(String, HcpIdent, HcpIdent)], sel: HcpIdent) -> Vec<NamedHcp> {
    pairs.iter().map(|(name, a_hcp, b_hcp)| {
        let width = arena.get_hw_bit_sz(a_hcp);
        let wire  = arena.make_wire(false, &format!("{}_RMUX", a_hcp.get_global_name()), width);
        mux_into_wire(arena, wire, *a_hcp, *b_hcp, sel);
        (name.clone(), wire)
    }).collect()
}

/// Pack the winner's `(name, hcp)` fields into a fresh wire-backed scalar Karray
/// (shape `[1]`) named `base`, returning its CcpIdent (the readable winner element).
pub fn reduce_pack(arena: &mut ModelArena, base: &str, fields: &[NamedHcp]) -> CcpIdent {
    let layout: Vec<(String, i32)> = fields.iter().map(|(n, h)| (n.clone(), arena.get_hw_bit_sz(h))).collect();
    let src   : Vec<HcpIdent>      = fields.iter().map(|(_, h)| *h).collect();
    pack_scalar_karray(base, layout, &src, arena)
}

// The `reduce_run` algorithm (recursive driver + helpers) lives in `karray_dynamic_reduce_get_run.rs`.