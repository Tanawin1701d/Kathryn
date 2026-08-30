// Python-facing Karray operations — mirrors the host `karray/arena_impl_ccp_karray.rs`.
// - Every method receives per-dim `KIdx` selectors encoded/decoded by
//   `kidx_py.rs` (the ONE place index kinds cross the boundary).
// - Source name / int-literal resolution lives HERE so the core stays name- and
//   BigInt-free: names resolve via `karray_fields`, int literals wrap into a
//   `val` sized to the matched field (`make_const_val`).
// - READ paths take `slf: &Bound<..>` (NOT &mut self): `PyKReadEnv` below
//   implements `KReadEnv` with SCOPED borrows, and each reduce-select callback
//   runs with NO borrow held — so the user's select fn may re-enter the arena.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::super::hw_component::common::operand_py::PyOperand;
use super::super::ccp_ident_py::PyCcpIdent;
use super::kidx_py::{decode_kidx, karray_err_to_py, warn_skipped_fields, PyKIdx};
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::KReadEnv;
use crate::model::controller::clock_mode::get_global_clk_mode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

#[pymethods]
impl PyModelArena {
    // ---- layout queries ----------------------------------------------------

    // The Karray's shape. The DSL enumerates a custom write-fn index over the
    // addressed dimension's extent with this (the Python handle caches no layout).
    fn karray_shape(&mut self, karray_i: PyCcpIdent) -> Vec<usize> {
        self.arena.karray_shape(karray_i.into())
    }

    // Whether the backing is clocked (reg → `|=`) vs combinational (wire → `*=`).
    fn karray_is_clocked(&mut self, karray_i: PyCcpIdent) -> bool {
        self.arena.karray_is_clocked(karray_i.into())
    }

    // The backing HCP of one field at a fully-static coordinate — reach an
    // element's own component to use the component's API on it (see the host
    // method; not on the reset path — that walk lives in karray_reset_field).
    fn karray_element_hcp(
        &mut self,
        karray_i: PyCcpIdent,
        coord   : Vec<usize>,
        field   : &str,
    ) -> PyResult<PyHcpIdent> {
        self.arena.karray_element_hcp(karray_i.into(), &coord, field)
            .map(Into::into)
            .map_err(karray_err_to_py)
    }

    // ---- reset -------------------------------------------------------------

    // Reset one field of a reg-backed Karray across EVERY element (the host walks
    // the field's backing regs). An int value wraps ONCE into a val sized to the
    // field — one val backs all elements rather than one per element.
    fn karray_reset_field(
        &mut self,
        karray_i: PyCcpIdent,
        field   : &str,
        value   : PyOperand,
    ) -> PyResult<()> {
        let karray_i: CcpIdent = karray_i.into();
        let reset_val_i: HcpIdent = match value {
            PyOperand::Ident(sig) => sig.into(),
            PyOperand::Int(n)     => {
                let width = self.arena.karray_field_width(karray_i, field)
                    .map_err(karray_err_to_py)?;
                self.make_const_val("kconst", &n, width)
            }
        };
        self.arena.karray_reset_field(karray_i, field, reset_val_i, get_global_clk_mode())
            .map_err(karray_err_to_py)
    }

    // ---- read --------------------------------------------------------------

    // Resolve a fully-collapsed selection + field NAME to that field's (possibly
    // muxed) HCP — the `d[i][sig].field` read target. `fns` carries one entry per
    // dimension: the wrapped reduce-select callable for a "reduce" selector, None
    // otherwise.
    fn karray_read_field_hcp(
        slf     : &Bound<'_, PyModelArena>,
        karray_i: PyCcpIdent,
        sel     : Vec<PyKIdx>,
        fns     : Vec<Option<Py<PyAny>>>,
        field   : &str,
    ) -> PyResult<PyHcpIdent> {
        let sel = decode_kidx(sel)?;
        let karray_i: CcpIdent = karray_i.into();

        let karray  = slf.borrow_mut().arena.take_karray(karray_i);
        let mut env = PyKReadEnv { slf, fns };
        let out     = karray.read_one_field(&sel, field, &mut env);
        slf.borrow_mut().arena.replace_back_karray(karray);
        out.map(Into::into)
    }

    // ---- assign ------------------------------------------------------------

    // Write scalar sources into the selected element(s). Each source pairs an
    // optional field name with a signal-or-int operand: `None` names the sole
    // field (single-field Karrays only), a `Some(name)` matching no field is
    // skipped with a warning, and an int wraps into a val sized to its field.
    fn karray_assign_hcps(
        &mut self,
        py            : Python<'_>,
        karray_i      : PyCcpIdent,
        sel           : Vec<PyKIdx>,
        sources       : Vec<(Option<String>, PyOperand)>,
        expect_clocked: bool,
    ) -> PyResult<()> {
        let sel    = decode_kidx(sel)?;
        let fields = self.arena.karray_fields(karray_i.into());

        let mut resolved: Vec<(usize, HcpIdent)> = Vec::with_capacity(sources.len());
        let mut skipped : Vec<String>            = Vec::new();
        for (name, src) in sources {
            let field_idx = match &name {
                None => {
                    if fields.len() != 1 {
                        return Err(PyTypeError::new_err(
                            "a bare scalar source can only drive a single-field Karray; \
                             use a {field_name: source} mapping"));
                    }
                    0
                }
                Some(field_name) => match fields.iter().position(|(fname, _)| fname == field_name) {
                    Some(field_idx) => field_idx,
                    None            => { skipped.push(field_name.clone()); continue; }
                }
            };
            let src_i: HcpIdent = match src {
                PyOperand::Ident(sig) => sig.into(),
                PyOperand::Int(n)     => self.make_const_val("kconst", &n, fields[field_idx].1),
            };
            resolved.push((field_idx, src_i));
        }
        if resolved.is_empty() {
            return Err(PyValueError::new_err("Karray assignment: no source matched any field"));
        }

        self.arena.karray_assign_hcps(karray_i.into(), &sel, &resolved, expect_clocked)
            .map_err(karray_err_to_py)?;
        warn_skipped_fields(py, "karray assign: skipped sources with no matching field", &skipped)
    }

    // Karray-to-karray region copy. Both sides pass per-dimension `KIdx`
    // selectors; kept (range) dims pair 1:1, fields pair by exact name+width,
    // and skipped destination fields raise a Python warning. `src_fns` carries
    // the SOURCE side's reduce-select callables (one entry per src dimension,
    // None for non-reduce dims); the destination side never has one (a custom
    // fn on a write destination arrives pre-evaluated as "cus" enables).
    fn karray_assign_k2k(
        slf           : &Bound<'_, PyModelArena>,
        dst_i         : PyCcpIdent,
        dst_sel       : Vec<PyKIdx>,
        src_i         : PyCcpIdent,
        src_sel       : Vec<PyKIdx>,
        src_fns       : Vec<Option<Py<PyAny>>>,
        expect_clocked: bool,
    ) -> PyResult<()> {
        let dst_sel = decode_kidx(dst_sel)?;
        let src_sel = decode_kidx(src_sel)?;
        let dst_i: CcpIdent = dst_i.into();
        let src_i: CcpIdent = src_i.into();

        // Take both karrays out (a single take when both sides are the same one),
        // run the engines borrow-free, then put them back — even on error.
        let (dst, src_owned) = {
            let mut me = slf.borrow_mut();
            let dst       = me.arena.take_karray(dst_i);
            let src_owned = if src_i == dst_i { None } else { Some(me.arena.take_karray(src_i)) };
            (dst, src_owned)
        };
        let src_ref = src_owned.as_ref().unwrap_or(&dst);

        // k2k = read → write composition. Destination guards run FIRST (zero
        // hardware) so a bad operator/backing errors before any source mux is
        // built or any user reduce fn fires.
        let out = (|| {
            dst.check_write_ok(&dst_sel, expect_clocked).map_err(karray_err_to_py)?;
            let src_field_idxs: Vec<usize> = (0..src_ref.field_count()).collect();
            let mut env = PyKReadEnv { slf, fns: src_fns };
            let view = src_ref.read_view(&src_sel, &src_field_idxs, &mut env)?;
            env.with_arena(|arena| dst.write(&dst_sel, &view, expect_clocked, arena))
                .map_err(karray_err_to_py)
        })();

        {
            let mut me = slf.borrow_mut();
            if let Some(src) = src_owned { me.arena.replace_back_karray(src); }
            me.arena.replace_back_karray(dst);
        }
        let skipped = out?;
        warn_skipped_fields(slf.py(), "karray-to-karray assign: skipped destination fields with no name+width match", &skipped)
    }
}

// ---- the Python read env -----------------------------------------------------

// KReadEnv over the arena pyclass: `with_arena` takes a SCOPED borrow (dropped on
// return); `reduce_select` calls the user's wrapped select fn with NO borrow held,
// so the fn may re-enter the arena to build its select expression.
struct PyKReadEnv<'a, 'py> {
    slf: &'a Bound<'py, PyModelArena>,
    fns: Vec<Option<Py<PyAny>>>,     // one per dimension; None for non-reduce dims
}

impl KReadEnv for PyKReadEnv<'_, '_> {
    type Err = PyErr;

    fn with_arena<R>(&mut self, f: impl FnOnce(&mut ModelArena) -> R) -> R {
        let mut me = self.slf.borrow_mut();
        f(&mut me.arena)
    }

    fn reduce_select(
        &mut self,
        dim      : usize,
        a_fields : &[(String, HcpIdent)], a_covered: &[usize],
        b_fields : &[(String, HcpIdent)], b_covered: &[usize],
        level    : u32,
    ) -> PyResult<(HcpIdent, Vec<(String, HcpIdent)>)> {
        let py  = self.slf.py();
        let raw = self.fns.get(dim).and_then(|f| f.as_ref())
            .ok_or_else(|| PyValueError::new_err(format!("Karray reduce: dimension {dim} has no select fn")))?;
        let args = (fields_to_py(a_fields), a_covered.to_vec(),
                    fields_to_py(b_fields), b_covered.to_vec(), level);
        let (sel, extras): (PyHcpIdent, Vec<(String, PyHcpIdent)>) = raw.bind(py).call1(args)?.extract()?;
        Ok((sel.into(), extras.into_iter().map(|(n, h)| (n, h.into())).collect()))
    }
}

// Convert (name, HcpIdent) fields into the Python-facing (name, PyHcpIdent) list.
fn fields_to_py(fields: &[(String, HcpIdent)]) -> Vec<(String, PyHcpIdent)> {
    fields.iter().map(|(name, hcp_i)| (name.clone(), (*hcp_i).into())).collect()
}
