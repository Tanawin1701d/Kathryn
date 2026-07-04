use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::complex_hardware::karray::karray_static_sel::KarrayAsmErr;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// Shared Karray hardware-build primitives — the one home for helpers used by more
// than one Karray operation file. Splitting them out keeps each op file (get /
// assign / reduce / cus_assign) focused on its own algorithm while the wiring
// primitives live in exactly one place.
//
//   - combinational mux / wire builders  (read + reduce paths)
//   - guarded-write primitives           (dynamic_assign + cus_dynamic_assign)
//   - single-node join                   (every assign path)

// ---- combinational mux / wire builders --------------------------------------

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
        bind_source_to_wire(arena, dst_i, src_i);
    }
    arena.replace_back_karray(res);
    res_ccp
}

// Mux two sources into `dest_w` and commit the muxed event straight onto the
// destination's update pool (no asm node — AssignMeta::mux + final_update).
pub(crate) fn mux_into_wire(arena: &mut ModelArena, dest_w: HcpIdent, l_src: HcpIdent, r_src: HcpIdent, select_left: HcpIdent) {
    let width  = arena.get_hw_bit_sz(&dest_w);
    let mut lm = build_comb_asm_meta(arena, dest_w, l_src, width);
    let mut rm = build_comb_asm_meta(arena, dest_w, r_src, width);
    let muxed  = lm.mux(&mut rm, select_left, arena);
    muxed.final_update(arena);
}

// One-hot select bit for index `idx`: `sig[idx]`. Shared by the dynamic-index mux
// tree (read) and the dynamic-assign write-enable path.
pub(crate) fn onehot_select_bit(sig_i: HcpIdent, idx: usize, arena: &mut ModelArena) -> HcpIdent {
    let bit = idx as i32;
    arena.make_expression_single(false, &format!("{}_OH{idx}", sig_i.get_global_name()),
        LogicOp::SliceBit, sig_i, Some(Slice::new(bit, bit + 1)))
}

// Binary equality write-enable for index `idx`: `sig == idx` (a 1-bit signal), with
// the constant sized to the dimension's index width. Used by the dynamic-assign path
// to enable exactly the runtime-selected element.
pub(crate) fn binary_index_eq(sig_i: HcpIdent, idx: usize, len: usize, arena: &mut ModelArena) -> HcpIdent {
    let iw      = index_width_for(len);
    let const_i = arena.make_val(false, &format!("{}_EQ{idx}", sig_i.get_global_name()), iw, idx as u64);
    arena.make_expression(false, &format!("{}_EQ{idx}_C", sig_i.get_global_name()),
        LogicOp::RelationEq, sig_i, const_i, None, None)
}

// Build a combinational AssignMeta (ClkUnused, so it can feed AssignMeta::mux)
// driving `dest_i` from `src_i` over the full destination width.
fn build_comb_asm_meta(arena: &mut ModelArena, dest_i: HcpIdent, src_i: HcpIdent, width: i32) -> AssignMeta {
    let des      = arena.take_hcp(dest_i);
    let priority = des.get_priority();
    let uei      = des.gen_update_event_with_pri(src_i, None, Slice::new(0, width), priority, ClockMode::ClkUnused, arena);
    let am       = AssignMeta::new(dest_i, uei, ClockMode::ClkUnused);
    arena.replace_back_hcp(des);
    am
}

// Bind a single source straight onto `dest_w`'s update pool (no asm node).
fn bind_source_to_wire(arena: &mut ModelArena, dest_w: HcpIdent, src_i: HcpIdent) {
    let width = arena.get_hw_bit_sz(&dest_w);
    let am    = build_comb_asm_meta(arena, dest_w, src_i, width);
    am.final_update(arena);
}

// ---- shared Karray assign primitives ----------------------------------------

impl Karray {
    /// Join a batch of per-field `AssignMeta`s into a SINGLE basic node named
    /// `<ccp>_<suffix>` and attach it to the current scope (no-op when empty). This is
    /// the one place a Karray turns several field writes into a single-cycle node — so
    /// a seq block advances once, not once per field. Shared by every Karray assign
    /// path: element (`elem_asm`), region (`karr_asm`), and dynamic (`dyn_asm`).
    pub fn attach_metas_as_node(&self, suffix: &str, metas: Vec<AssignMeta>, arena: &mut ModelArena) {
        if !metas.is_empty() {
            let name   = format!("{}_{suffix}", self.get_ccp_ident().get_global_name());
            let node_i = arena.make_asm_node_many(&name, &metas);
            arena.attach_basic_node_to_current_scope(node_i);
        }
    }

    /// Emit the guarded writes for ONE fully-pinned element `coord`: one
    /// `AssignMeta` per matched field, each driving the field's HCP from its source
    /// and (when `we` is given) parking that write-enable for build-time guarding.
    /// Pushes them onto `metas` (joined into a single node later). Shared by the
    /// auto-index dynamic write and the callback-driven cus_dynamic_assign.
    pub fn gen_element_asm_metas(
        &self,
        coord       : &[usize],
        resolved_src: &[(usize, HcpIdent)],
        we          : Option<HcpIdent>,
        metas       : &mut Vec<AssignMeta>,
        arena       : &mut ModelArena,
    ) {
        for &(field_idx, src_i) in resolved_src {
            let des_i             = self.static_index_get_hcp(coord, field_idx, false, arena);
            let src_full_sl       = arena.get_hw_slice(&src_i);
            let (mut am, _resize) = arena.gen_asm_meta(des_i, src_i, None, src_full_sl);
            if let Some(c) = we { am.set_pending_pre_cond(c); }
            metas.push(am);
        }
    }

    /// Reject combinational intent and any non-Reg backing for a dynamic write.
    /// `clocked` is the resolved operator intent (`|=` → true, `*=` → false, `=` →
    /// the backing's own clocked-ness), so a `false` here means a combinational write.
    pub(crate) fn assert_dynamic_write_backing(&self, clocked: bool) -> Result<(), KarrayAsmErr> {
        if !clocked {
            return Err(KarrayAsmErr::Type(
                "combinational dynamic Karray write is not supported; use `|=` on a reg-backed Karray".into()));
        }
        if !matches!(self.get_backing(), HwComponentType::Reg) {
            return Err(KarrayAsmErr::Type(format!(
                "dynamic Karray assignment requires a reg-backed Karray, got {:?}", self.get_backing())));
        }
        Ok(())
    }

    /// Resolve `(field_name, src_i)` sources to `(field_idx, src_i)`; names matching
    /// no field are collected for a caller warning.
    pub(crate) fn match_sources_to_fields(&self, sources: &[(String, HcpIdent)]) -> (Vec<(usize, HcpIdent)>, Vec<String>) {
        let mut resolved = Vec::with_capacity(sources.len());
        let mut skipped  = Vec::new();
        for (name, src_i) in sources {
            match self.field_index(name) {
                Some(field_idx) => resolved.push((field_idx, *src_i)),
                None            => skipped.push(name.clone()),
            }
        }
        (resolved, skipped)
    }
}
