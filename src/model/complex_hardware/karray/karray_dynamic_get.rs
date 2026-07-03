use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::Karray;
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::complex_hardware::karray::karray_dyn_sel::{DynReduc, DynSelKarray, KyIdxType};

impl Karray{

    // Recursively resolve a mixed static/dynamic index into a single element's
    // mux wires. Static dims pin and descend; a dynamic dim fans out over every
    // index, recurses, then folds the children into a balanced 2:1 mux tree.
    pub fn dynamic_index_base(&self, dim_idx        : usize,
                                     fixed_indices  : &mut Vec<usize>,
                                     abs_indices    : &mut Vec<KyIdxType>,
                                     selected_field : &mut Vec<usize>, // field_idx in backing_hcps
                                     request_index  : bool,
                                     arena          : &mut ModelArena ) -> DynSelKarray {

        // ---- base case: every dim pinned -> read this element's field HCPs ----
        if dim_idx == self.get_dim_size(){
            return self.build_leaf_dyn_sel(fixed_indices, selected_field, arena);
        }

        // ---- pick the selector for this dim (Static descends in place) ----
        let orig_sel          = abs_indices[dim_idx];
        let (sig_i, is_onehot) = match orig_sel {
            KyIdxType::Static(fixed_idx) => {
                // one child only: pin this dim to fixed_idx, recurse to dim_idx + 1
                fixed_indices.push(fixed_idx);
                let mut res = self.dynamic_index_base(dim_idx + 1, fixed_indices, abs_indices, selected_field, request_index, arena);
                fixed_indices.pop();
                res.result_indexed_i[dim_idx] = KyIdxType::Static(fixed_idx);
                return res;
            }
            KyIdxType::DynBin   (sig_i) => (sig_i, false),
            KyIdxType::DynOneHot(sig_i) => (sig_i, true),
        };

        ////// loop to all possible index in dimension dim_idx and call dynamic_index_base to get DynSelKarray
        let len = self.get_shape()[dim_idx];

        // the runtime selector must be wide enough to address every element of this dim:
        // a binary address needs ceil(log2(len)) bits, a one-hot line needs one bit per index.
        let sig_w = arena.get_hw_bit_sz(&sig_i);
        if is_onehot {
            assert!(sig_w >= len as i32,
                "dynamic_index_base: one-hot selector for dim {dim_idx} needs >= {len} bits, got {sig_w}");
        } else {
            let need = index_width_for(len);
            assert!(sig_w >= need,
                "dynamic_index_base: binary selector for dim {dim_idx} needs >= {need} bits to index {len} elements, got {sig_w}");
        }

        let mut children  = Vec::with_capacity(len);
        for i in 0..len {
            fixed_indices.push(i);
            let child = self.dynamic_index_base(dim_idx + 1,
                                                             fixed_indices, abs_indices,
                                                             selected_field,
                                                             request_index,
                                                             arena);
            children.push(child);
            fixed_indices.pop();
        }

        ////// build wire that compose the binary mux tree
        ////// compose DynSelKarray
        self.compose_dyn_mux(dim_idx, sig_i, is_onehot, children, request_index, arena)
    }

    // Leaf: every dimension is pinned in `fixed_indices`, so read the backing HCP
    // of each selected field directly and record the (all-static) coordinate.
    fn build_leaf_dyn_sel(&self, fixed_indices: &[usize], selected_field: &[usize], arena: &mut ModelArena) -> DynSelKarray {
        let mut immediate_hcps = Vec::with_capacity(selected_field.len());
        let mut field_names    = Vec::with_capacity(selected_field.len());
        for &field_idx in selected_field {
            immediate_hcps.push(self.static_index_get_hcp(fixed_indices, field_idx, true, arena));
            field_names   .push(self.get_fields()[field_idx].get_name().to_string());
        }
        // one coordinate slot per dimension, pre-filled with the "unresolved" sentinel;
        // each dim's real index is stamped in by the static/dynamic handlers above.
        let result_indexed_i = vec![KyIdxType::Static(usize::MAX); fixed_indices.len()];
        DynSelKarray { immediate_hcps, field_names, result_indexed_i }
    }

    // Fold the per-index children of dimension `dim_idx` into one element by a
    // balanced binary mux tree, then stamp the resolved coordinate for this dim.
    fn compose_dyn_mux(&self, dim_idx       : usize,
                              sig_i         : HcpIdent,
                              is_onehot     : bool,
                              children      : Vec<DynSelKarray>,
                              request_index : bool,
                              arena         : &mut ModelArena ) -> DynSelKarray {

        let field_names = children[0].field_names.clone();
        let base_coords = children[0].result_indexed_i.clone();   // deeper/shallower coords template
        let idx_w       = index_width_for(self.get_shape()[dim_idx]);
        let base        = self.get_ccp_ident().get_global_name().to_string();

        // ---- leaf reduction nodes: data wires + the literal index value ----
        let mut nodes: Vec<DynReduc> = Vec::with_capacity(children.len());
        for (i, child) in children.into_iter().enumerate() {
            let idx_i = request_index.then(||
                arena.make_val(false, &format!("{base}_DIDX_D{dim_idx}_{i}"), idx_w, i as u64));
            let oh_sel_i = is_onehot.then(|| onehot_bit(sig_i, i, arena));   // leaf: this index's one-hot bit
            nodes.push(DynReduc { data_i: child.immediate_hcps, idx_i, oh_sel_i });
        }

        // ---- balanced pairwise reduction (2:1 muxes) ----
        let mut layer = 0u32;
        while nodes.len() > 1 {
            // Binary: ONE shared select bit per tree level — `sig[layer]` distinguishes
            // the two children at every node of this level (so an N-bit address drives
            // an N-deep tree). One-hot has no such per-level bit; it selects per node
            // from each subtree's covered indices (computed inside create_mux).
            let layer_sel = (!is_onehot).then(|| bin_layer_select_left(sig_i, layer, arena));
            let mut next  = Vec::with_capacity(nodes.len().div_ceil(2));
            let mut iter  = nodes.into_iter();
            while let Some(left) = iter.next() {
                match iter.next() {
                    Some(right) => next.push(create_mux(left, right, layer_sel, arena)),
                    None        => next.push(left),   // odd one out — carried up unchanged
                }
            }
            nodes  = next;
            layer += 1;
        }
        let final_node = nodes.pop().expect("a dynamic dimension has at least one element");

        // ---- compose: replace this dim's coordinate with the resolved index ----
        let mut result_indexed_i = base_coords;
        result_indexed_i[dim_idx] = match final_node.idx_i {
            Some(idx_i) => KyIdxType::DynBin(idx_i),   // request_index: carry the chosen index
            None        => KyIdxType::Static(usize::MAX),   // sentinel: index not resolved
        };
        DynSelKarray { immediate_hcps: final_node.data_i, field_names, result_indexed_i }
    }

    /// Read a dynamically-indexed element into a fresh wire-backed scalar Karray
    /// (shape `[1]`) holding the selected fields. Returns the result CcpIdent and
    /// the resolved per-dimension index (`DynBin` wire for dynamic dims, `Static`
    /// for pinned ones). Only Reg/Wire backings are supported (MemBlock is meant
    /// to be addressed directly, not muxed element-by-element).
    #[allow(non_snake_case)]
    pub fn dynamic_index_get_Karray(&self, indices: &[KyIdxType], selected_fields: Vec<usize>, arena: &mut ModelArena) -> (CcpIdent, Vec<KyIdxType>){
        assert!(matches!(self.get_backing(), HwComponentType::Reg | HwComponentType::Wire),
            "dynamic_index_get_Karray: only Reg- or Wire-backed Karrays support dynamic indexing, got {:?}",
            self.get_backing());
        assert_eq!(indices.len(), self.get_dim_size(),
            "dynamic_index_get_Karray: expected {} index selectors (one per dimension), got {}",
            self.get_dim_size(), indices.len());
        let nf = self.field_count();
        assert!(selected_fields.iter().all(|&fi| fi < nf),
            "dynamic_index_get_Karray: selected field index out of range (have {nf} fields): {selected_fields:?}");

        // ---- resolve the dynamic selection into mux-output wires ----
        let mut fixed_indices = Vec::with_capacity(self.get_dim_size());
        let mut abs_indices   = indices.to_vec();
        let mut sel_fields    = selected_fields.clone();
        let sel = self.dynamic_index_base(0, &mut fixed_indices, &mut abs_indices, &mut sel_fields, true, arena);

        // ---- materialise a wire-backed scalar result Karray (shape [1]) ----
        let fields: Vec<(String, i32)> = selected_fields.iter()
            .map(|&fi| (self.get_fields()[fi].get_name().to_string(), self.get_fields()[fi].get_width()))
            .collect();
        let name    = format!("{}_DYNSEL", self.get_ccp_ident().get_global_name());
        let res_ccp = pack_scalar_karray(&name, fields, &sel.immediate_hcps, arena);
        (res_ccp, sel.result_indexed_i)
    }
}

// Pack a row of per-field source signals into a fresh wire-backed scalar Karray
// (shape [1], named `name`) and drive each field combinationally from its source
// (parallel to `src_hcps`). Shared by the dynamic-index read and the reduce.
pub(crate) fn pack_scalar_karray(
    name     : &str,
    fields   : Vec<(String, i32)>,
    src_hcps : &[HcpIdent],
    arena    : &mut ModelArena,
) -> CcpIdent {
    let res_ccp = arena.make_karray(false, name, vec![1], fields, HwComponentType::Wire);
    let res     = arena.take_karray(res_ccp);
    for (field_idx, &src_i) in src_hcps.iter().enumerate() {
        let dst_i = res.static_index_get_hcp(&[0], field_idx, false, arena);
        dry_bind(arena, dst_i, src_i);
    }
    arena.replace_back_karray(res);
    res_ccp
}

// ---- mux primitives (combinational, arena-only) -----------------------------

// Mux two reduction nodes: select the left subtree when the runtime selector
// favours it, else the right. Each field (and the optional carried index) gets a
// fresh wire driven by an AssignMeta::mux UeCond. `layer_sel` is the binary path's
// precomputed per-level select bit; `None` means one-hot, where the select line is
// the OR of the left subtree's own one-hot bits.
fn create_mux(left: DynReduc, right: DynReduc, layer_sel: Option<HcpIdent>, arena: &mut ModelArena) -> DynReduc {
    // select_left + the merged select line carried up to the parent.
    let (select_left, next_oh_sel_i) = match layer_sel {
        Some(bit) => (bit, None),   // binary: shared per-level address bit; nothing carried
        None      => {              // one-hot: left's running OR IS the select; merge for the parent
            let ls     = left .oh_sel_i.expect("one-hot reduction node carries its select OR");
            let rs     = right.oh_sel_i.expect("one-hot reduction node carries its select OR");
            let merged = arena.make_expression(false, &format!("{}_SOR", ls.get_global_name()),
                LogicOp::BitwiseOr, ls, rs, None, None);
            (ls, Some(merged))
        }
    };

    // mux each selected field's value into a fresh wire
    let mut data_i = Vec::with_capacity(left.data_i.len());
    for f in 0..left.data_i.len() {
        let width = arena.get_hw_bit_sz(&left.data_i[f]);
        let res_w = arena.make_wire(false, &format!("{}_DMUX", left.data_i[f].get_global_name()), width);
        mux_into_wire(arena, res_w, left.data_i[f], right.data_i[f], select_left);
        data_i.push(res_w);
    }

    // mux the carried index, when requested
    let idx_i = match (left.idx_i, right.idx_i) {
        (Some(li), Some(ri)) => {
            let width = arena.get_hw_bit_sz(&li);
            let res_w = arena.make_wire(false, &format!("{}_DMUXIDX", li.get_global_name()), width);
            mux_into_wire(arena, res_w, li, ri, select_left);
            Some(res_w)
        }
        _ => None,
    };

    DynReduc { data_i, idx_i, oh_sel_i: next_oh_sel_i }
}

// One-hot select bit for index `idx`: `sig[idx]`. Shared with the dynamic-assign
// write-enable path (karray_dynamic_assign.rs).
pub(crate) fn onehot_bit(sig_i: HcpIdent, idx: usize, arena: &mut ModelArena) -> HcpIdent {
    let bit = idx as i32;
    arena.make_expression_single(false, &format!("{}_OH{idx}", sig_i.get_global_name()),
        LogicOp::SliceBit, sig_i, Some(Slice::new(bit, bit + 1)))
}

// Binary equality write-enable for index `idx`: `sig == idx` (a 1-bit signal), with
// the constant sized to the dimension's index width. Used by the dynamic-assign path
// to enable exactly the runtime-selected element.
pub(crate) fn bin_eq(sig_i: HcpIdent, idx: usize, len: usize, arena: &mut ModelArena) -> HcpIdent {
    let iw      = index_width_for(len);
    let const_i = arena.make_val(false, &format!("{}_EQ{idx}", sig_i.get_global_name()), iw, idx as u64);
    arena.make_expression(false, &format!("{}_EQ{idx}_C", sig_i.get_global_name()),
        LogicOp::RelationEq, sig_i, const_i, None, None)
}

// Binary per-level select-left bit: at tree level `layer`, the two children of a
// node differ in bit `layer` of the address (left = 0, right = 1), so pick left
// when `sig[layer] == 0`, i.e. `~sig[layer]`.
fn bin_layer_select_left(sig_i: HcpIdent, layer: u32, arena: &mut ModelArena) -> HcpIdent {
    let bit     = layer as i32;
    let bit_sig = arena.make_expression_single(false, &format!("{}_B{layer}", sig_i.get_global_name()),
        LogicOp::SliceBit, sig_i, Some(Slice::new(bit, bit + 1)));
    arena.make_expression_single(false, &format!("{}_B{layer}_N", sig_i.get_global_name()),
        LogicOp::BitwiseInvr, bit_sig, None)
}

// Build a combinational AssignMeta (ClkUnused, so it can feed AssignMeta::mux)
// driving `dest_i` from `src_i` over the full destination width.
fn create_wire_asm_meta(arena: &mut ModelArena, dest_i: HcpIdent, src_i: HcpIdent, width: i32) -> AssignMeta {
    let des      = arena.take_hcp(dest_i);
    let priority = des.get_priority();
    let uei      = des.gen_update_event_with_pri(src_i, None, Slice::new(0, width), priority, ClockMode::ClkUnused, arena);
    let am       = AssignMeta::new(dest_i, uei, ClockMode::ClkUnused);
    arena.replace_back_hcp(des);
    am
}

// Mux two sources into `dest_w` and commit the muxed event straight onto the
// destination's update pool (no asm node — AssignMeta::mux + final_update).
pub(crate) fn mux_into_wire(arena: &mut ModelArena, dest_w: HcpIdent, l_src: HcpIdent, r_src: HcpIdent, select_left: HcpIdent) {
    let width  = arena.get_hw_bit_sz(&dest_w);
    let mut lm = create_wire_asm_meta(arena, dest_w, l_src, width);
    let mut rm = create_wire_asm_meta(arena, dest_w, r_src, width);
    let muxed  = lm.mux(&mut rm, select_left, arena);
    muxed.final_update(arena);
}

// Bind a single source straight onto `dest_w`'s update pool (no asm node).
fn dry_bind(arena: &mut ModelArena, dest_w: HcpIdent, src_i: HcpIdent) {
    let width = arena.get_hw_bit_sz(&dest_w);
    let am    = create_wire_asm_meta(arena, dest_w, src_i, width);
    am.final_update(arena);
}
