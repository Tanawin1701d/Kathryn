use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_hw_build::{and_we, binary_index_eq};
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::complex_hardware::karray::karray_view::{KView, KViewPairing};
use crate::model::complex_hardware::karray::kidx::{check_kidx, has_runtime, KarrayErr, KIdx};
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// ===== Karray WRITE engine ====================================================
//
// `write` is THE statement entry: it owns the operator/backing guards, field
// pairing (per the source KView's stamped policy), and the ONE-joined-node
// rule. Below it, the fan-out machinery turns the selection into guarded
// per-element writes. Dims by kind:
//   * `Static` — descends into its single element.
//   * `Dyn`    — iterates every index, ANDing `sig == i` into the running
//                write-enable so only the runtime-selected element takes the write.
//   * `CusWe`  — iterates every index, ANDing that index's user-built enable bit
//                (the DSL pre-evaluated the user's fn(i) per index).
// At each fully-pinned coordinate one `AssignMeta` per source field is emitted
// with the accumulated enable parked as a pending pre-condition (folded in at
// build time, once the clk is wired — see `gen_element_asm_metas`).
//
// Because a non-selected element must HOLD, any selection containing a runtime
// dim (Dyn/Cus) requires a reg backing — an unfired clocked event keeps its
// value, a wire cannot.

impl Karray {
    // ---- statement guards --------------------------------------------------

    // Operator guard shared by every write path: `|=` (true) needs the clocked
    // (reg) backing, `*=` (false) the combinational (wire) one.
    pub fn check_operator_guard(&self, expect_clocked: bool) -> Result<(), KarrayErr> {
        match expect_clocked {
            true  if !self.is_clocked() => Err(KarrayErr::Type(
                "`|=` (clocked assign) requires a reg-backed Karray".into())),
            false if  self.is_clocked() => Err(KarrayErr::Type(
                "`*=` (combinational assign) requires a wire-backed Karray".into())),
            _ => Ok(()),
        }
    }

    // A runtime-collapsed (Dyn/Cus) write needs a reg destination: a non-selected
    // element HOLDS because its unfired clocked event keeps its value — a wire
    // cannot hold (it would be a mux-with-feedback).
    fn check_runtime_write(&self, sel: &[KIdx]) -> Result<(), KarrayErr> {
        if has_runtime(sel) && !self.is_clocked() {
            return Err(KarrayErr::Type(
                "a dynamic (signal / custom-fn indexed) Karray write requires a reg-backed \
                 Karray; a wire cannot hold its non-selected elements".into()));
        }
        Ok(())
    }

    /// k2k pre-flight: every destination guard, ZERO hardware. Run before the
    /// source side is read so a bad operator/backing errors before any source
    /// muxes are built or any user reduce fn fires.
    pub fn check_write_ok(&self, sel: &[KIdx], expect_clocked: bool) -> Result<(), KarrayErr> {
        self.check_operator_guard(expect_clocked)?;
        check_kidx(self.get_shape(), sel)?;
        self.check_runtime_write(sel)
    }

    // ---- THE write entry ---------------------------------------------------

    /// Write a source `KView` into the element selected by `sel`. Owns all
    /// statement policy: guards, field pairing per the view's stamped policy,
    /// and the join of every emitted meta into ONE basic node. Returns the
    /// destination field names skipped for want of a pairing match (Exact
    /// views only).
    pub fn write(
        &self,
        sel           : &[KIdx],
        view          : &KView,
        expect_clocked: bool,
        arena         : &mut ModelArena,
    ) -> Result<Vec<String>, KarrayErr> {
        // Step 1: operator + runtime-write guards.
        self.check_operator_guard(expect_clocked)?;
        check_kidx(self.get_shape(), sel)?;
        self.check_runtime_write(sel)?;

        // Step 2: pair destination fields against view fields, per the policy
        //         the view's constructor stamped.
        let mut matched: Vec<(usize, usize)> = Vec::new();   // (dst_field_idx, view_field_idx)
        let mut skipped: Vec<String>         = Vec::new();
        match view.get_pairing() {
            // k2k: exact name + width; a dst field with no counterpart is
            // skipped and reported for a caller warning.
            KViewPairing::Exact => {
                for (dst_field_idx, des_field) in self.get_fields().iter().enumerate() {
                    match view.get_fields().iter().position(|(name, width)|
                        name == des_field.get_name() && *width == des_field.get_width()) {
                        Some(view_field_idx) => matched.push((dst_field_idx, view_field_idx)),
                        None                 => skipped.push(des_field.get_name().to_string()),
                    }
                }
            }
            // Scalar sources: names are already canonical (connector-resolved
            // against THIS karray), so pair by name; widths auto-resize in
            // gen_asm_meta. Unmatched-source warnings stayed in the connector.
            KViewPairing::Named => {
                for (view_field_idx, (name, _)) in view.get_fields().iter().enumerate() {
                    if let Some(dst_field_idx) = self.field_index(name) {
                        matched.push((dst_field_idx, view_field_idx));
                    }
                }
            }
        }
        if matched.is_empty() {
            return Err(KarrayErr::Value(
                "Karray assignment: no destination field matches a source field by name+width".into()));
        }

        // Step 3: fan out the guarded writes, then join them into ONE basic
        //         node (suffix preserves the per-path node names).
        let resolved: Vec<(usize, HcpIdent)> = matched.iter()
            .map(|&(dst_field_idx, view_field_idx)| (dst_field_idx, view.hcp(view_field_idx)))
            .collect();
        let mut metas = Vec::new();
        self.write_metas(sel, &resolved, &mut metas, arena)?;

        let suffix = match view.get_pairing() {
            KViewPairing::Named => "elem_asm",
            KViewPairing::Exact => "karr_asm",
        };
        self.attach_metas_as_node(suffix, metas, arena);
        Ok(skipped)
    }

    // ---- fan-out machinery -------------------------------------------------

    /// Fan `resolved_src` (`(field_idx, src_i)` pairs) out over the validated
    /// selection, pushing one guarded `AssignMeta` per (element, field) onto
    /// `metas`. The caller joins them into a single node.
    pub(crate) fn write_metas(
        &self,
        // ---- static — fixed for the whole walk ----
        sel         : &[KIdx],
        resolved_src: &[(usize, HcpIdent)],
        // ---- sinks — output accumulator + arena context ----
        metas       : &mut Vec<AssignMeta>,
        arena       : &mut ModelArena,
    ) -> Result<(), KarrayErr> {
        let mut coord = Vec::with_capacity(self.get_dim_count());
        self.write_dim(0, &mut coord, None, sel, resolved_src, metas, arena)
    }

    // Recursive fan-out over dimension `dim_idx`; `we_acc` carries the ANDed
    // write-enable accumulated from the runtime dims above.
    #[allow(clippy::too_many_arguments)]
    fn write_dim(
        &self,
        // ---- iterating — advances per recursion level ----
        dim_idx     : usize,
        coord       : &mut Vec<usize>,
        we_acc      : Option<HcpIdent>,
        // ---- static — fixed for the whole walk ----
        sel         : &[KIdx],
        resolved_src: &[(usize, HcpIdent)],
        // ---- sinks — output accumulator + arena context ----
        metas       : &mut Vec<AssignMeta>,
        arena       : &mut ModelArena,
    ) -> Result<(), KarrayErr> {
        // ---- base case: every dim pinned -> one guarded write per source field ----
        if dim_idx == self.get_dim_count() {
            self.gen_element_asm_metas(coord, resolved_src, we_acc, metas, arena);
            return Ok(());
        }

        let len = self.get_shape()[dim_idx];
        match &sel[dim_idx] {
            KIdx::Static(fixed_idx) => {
                coord.push(*fixed_idx);
                self.write_dim(dim_idx + 1, coord, we_acc, sel, resolved_src, metas, arena)?;
                coord.pop();
            }

            KIdx::Dyn(sig_i) => {
                let need = index_width_for(len);
                let got  = arena.get_hw_bit_sz(sig_i);
                if got < need {
                    return Err(KarrayErr::Value(format!(
                        "dynamic index for dim {dim_idx} needs >= {need} bits to address {len} elements, got {got}")));
                }
                for i in 0..len {
                    let we_i = binary_index_eq(*sig_i, i, len, arena);
                    let we   = and_we(we_acc, we_i, arena);
                    coord.push(i);
                    self.write_dim(dim_idx + 1, coord, we, sel, resolved_src, metas, arena)?;
                    coord.pop();
                }
            }

            KIdx::CusWe(bits) => {
                let bits = bits.clone();   // detach from sel so the recursion may re-borrow it
                for (i, &bit_i) in bits.iter().enumerate() {
                    let we = and_we(we_acc, bit_i, arena);
                    coord.push(i);
                    self.write_dim(dim_idx + 1, coord, we, sel, resolved_src, metas, arena)?;
                    coord.pop();
                }
            }

            KIdx::CusRd => {
                return Err(KarrayErr::Type(
                    "a reduce (read-side custom) index cannot be a write destination; \
                     on the write side a custom fn is a per-index enable (fn(i) -> 1-bit)".into()));
            }
        }
        Ok(())
    }
}
