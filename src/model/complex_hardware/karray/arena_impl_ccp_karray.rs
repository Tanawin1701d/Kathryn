use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_env::DirectKEnv;
use crate::model::complex_hardware::karray::karray_view::KView;
use crate::model::complex_hardware::karray::kidx::{KarrayErr, KIdx};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// Higher-level Karray arena operations — thin PROXIES that take the Karray(s)
// out of the arena and drive the two engines (`Karray::read_view` /
// `Karray::write`); a k2k copy is their COMPOSITION, done right here. Plus the
// small layout queries the DSL/connector need (shape for custom-fn enumeration,
// fields for name/width resolution, clocked-ness for the operator guard).

impl ModelArena {
    // ---- scoped access -----------------------------------------------------

    // Scoped take/replace_back: run `f` with the taken Karray plus the freed
    // arena borrow, then put the Karray back. Every proxy below routes through
    // this so the sandwich lives in ONE place.
    fn with_karray<R>(
        &mut self,
        karray_i: CcpIdent,
        f       : impl FnOnce(&Karray, &mut ModelArena) -> R,
    ) -> R {
        let karray = self.take_karray(karray_i);
        let out    = f(&karray, self);
        self.replace_back_karray(karray);
        out
    }

    // ---- layout queries ----------------------------------------------------

    // The Karray's shape (one extent per dimension). The DSL enumerates a
    // custom-fn index over the addressed dimension's extent with this.
    pub fn karray_shape(&mut self, karray_i: CcpIdent) -> Vec<usize> {
        self.with_karray(karray_i, |karray, _| karray.get_shape().clone())
    }

    // The element record as `(name, width)` pairs, in declaration order. The
    // connector resolves source field names and int-literal widths against this.
    pub fn karray_fields(&mut self, karray_i: CcpIdent) -> Vec<(String, i32)> {
        self.with_karray(karray_i, |karray, _| karray.get_fields().iter()
            .map(|f| (f.get_name().to_string(), f.get_width()))
            .collect())
    }

    // Whether the backing is clocked (reg → `|=`) or combinational (wire → `*=`).
    pub fn karray_is_clocked(&mut self, karray_i: CcpIdent) -> bool {
        self.with_karray(karray_i, |karray, _| karray.is_clocked())
    }

    // The backing HCP of ONE field at a FULLY-STATIC coordinate — the leaf a read
    // or write of that element would resolve to. Pure lookup, no hardware.
    // It exists so a caller can reach an element's own component and use the
    // component's API on it (the DSL's `Karray.reset`): the reset value, its
    // priority and its clock stay the Reg's, and no Karray-specific reset
    // mechanism is added here. Static only — there is no runtime element to
    // hand back for a Dyn/Cus selection.
    pub fn karray_element_hcp(
        &mut self,
        karray_i: CcpIdent,
        coord   : &[usize],
        field   : &str,
    ) -> Result<HcpIdent, KarrayErr> {
        self.with_karray(karray_i, |karray, _| {
            let shape = karray.get_shape();
            if coord.len() != shape.len() {
                return Err(KarrayErr::Value(format!(
                    "a Karray coordinate must name every dimension: got {} for a {}-D Karray",
                    coord.len(), shape.len())));
            }
            for (dim_idx, (&dim_sz, &idx)) in shape.iter().zip(coord.iter()).enumerate() {
                if idx >= dim_sz {
                    return Err(KarrayErr::Value(format!(
                        "index {idx} out of bounds for dimension {dim_idx} of size {dim_sz}")));
                }
            }
            match karray.field_index(field) {
                Some(field_idx) => Ok(karray.element_hcp(coord, field_idx)),
                None            => Err(KarrayErr::Value(format!(
                    "Karray has no field '{field}' (fields: {})",
                    karray.get_fields().iter()
                          .map(|f| f.get_name()).collect::<Vec<_>>().join(", ")))),
            }
        })
    }

    // ---- read --------------------------------------------------------------

    // Resolve a fully-collapsed selection + field NAME to that field's (possibly
    // muxed) HCP — the target `d[i][sig].field` reads from. Runs under the
    // Rust-native env, so a reduce (`CusRd`) dim errors here — reduce selects
    // are driven through the Python connector's env.
    pub fn karray_read_field_hcp(
        &mut self,
        karray_i: CcpIdent,
        sel     : &[KIdx],
        field   : &str,
    ) -> Result<HcpIdent, KarrayErr> {
        self.with_karray(karray_i, |karray, arena| {
            let mut env = DirectKEnv { arena };
            karray.read_one_field(sel, field, &mut env)
        })
    }

    // ---- assign (read → write composition) ---------------------------------

    // Write scalar sources (`(field_idx, src_i)` pairs, names already resolved by
    // the connector) into the elements selected by `sel`: wrap them into a
    // rank-0 KView and hand it to the write engine.
    pub fn karray_assign_hcps(
        &mut self,
        karray_i      : CcpIdent,
        sel           : &[KIdx],
        sources       : &[(usize, HcpIdent)],
        expect_clocked: bool,
    ) -> Result<(), KarrayErr> {
        self.with_karray(karray_i, |karray, arena|
            KView::from_sources(karray, sources)
                .and_then(|view| karray.write(sel, &view, expect_clocked, arena))
                .map(|_skipped| ()))   // Named views pair canonically — never skip
    }

    // Region-to-region (k2k) copy proxy: READ the source region into a shaped
    // view, then WRITE it into the destination — k2k is composition, not an
    // engine. Takes dst out (reusing it as the source when both sides are the
    // same Karray — a single take avoids the double-take debug assert).
    pub fn karray_assign_k2k(
        &mut self,
        dst_i         : CcpIdent,
        dst_sel       : &[KIdx],
        src_i         : CcpIdent,
        src_sel       : &[KIdx],
        expect_clocked: bool,
    ) -> Result<Vec<String>, KarrayErr> {
        self.with_karray(dst_i, |dst, arena| {
            let src_owned = if src_i == dst_i { None } else { Some(arena.take_karray(src_i)) };
            let src_ref   = src_owned.as_ref().unwrap_or(dst);

            // Pre-flight the destination guards BEFORE building any source hardware
            // (preserves the old error order), then compose read → write.
            let out = dst.check_write_ok(dst_sel, expect_clocked).and_then(|_| {
                let src_field_idxs: Vec<usize> = (0..src_ref.field_count()).collect();
                let view = {
                    let mut env = DirectKEnv { arena: &mut *arena };
                    src_ref.read_view(src_sel, &src_field_idxs, &mut env)?
                };
                dst.write(dst_sel, &view, expect_clocked, arena)
            });

            if let Some(src) = src_owned { arena.replace_back_karray(src); }
            out
        })
    }
}
