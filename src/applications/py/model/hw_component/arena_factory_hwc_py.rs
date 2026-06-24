// Python-facing hardware-component factories. Mirrors the host
// `arena_factory_hwc.rs` split — every HWC `mk_*` user constructor lives here
// in its own `#[pymethods]` block (enabled by PyO3's `multiple-pymethods`).
// All wrappers are user-declared, so they pass `is_user_com = true`.

use pyo3::prelude::*;
use super::super::model_arena::PyModelArena;
use super::common::hcp_ident_py::PyHcpIdent;
use crate::util::math::vary_val::VaryVal;

#[pymethods]
impl PyModelArena {
    // ---- Reg ----------------------------------------------------------------

    // Declare a user register `bit_width` bits wide; returns its handle.
    fn mk_reg(&mut self, name: &str, bit_width: i32) -> PyHcpIdent {
        self.arena.make_reg(true, name, bit_width).into()
    }

    // ---- Wire ---------------------------------------------------------------

    // Declare a user wire `bit_width` bits wide; returns its handle.
    fn mk_wire(&mut self, name: &str, bit_width: i32) -> PyHcpIdent {
        self.arena.make_wire(true, name, bit_width).into()
    }

    // ---- Val ----------------------------------------------------------------

    // Declare a user constant value (≤64 bits) with literal `init_val`.
    fn mk_val(&mut self, name: &str, bit_width: i32, init_val: u64) -> PyHcpIdent {
        self.arena.make_val(true, name, bit_width, init_val).into()
    }

    // Declare a user constant value of arbitrary width from little-endian u64 limbs
    // (limbs[0] = bits 0..63, etc.) — the path for values wider than a single u64.
    fn mk_val_vv(&mut self, name: &str, bit_width: i32, limbs: Vec<u64>) -> PyHcpIdent {
        let vv = VaryVal::from_limbs(limbs, bit_width as usize);
        self.arena.make_val_vv(true, name, bit_width, vv).into()
    }

    // ---- MemBlk -------------------------------------------------------------

    // Declare a user memory block of `bit_width`-wide cells indexed by `index_width`.
    fn mk_mem_blk(&mut self, name: &str, bit_width: i32, index_width: i32) -> PyHcpIdent {
        self.arena.make_mem_blk(true, name, bit_width, index_width).into()
    }

    // ---- MemEle -------------------------------------------------------------

    // Declare a user memory element addressing `master_mem_blk_i` at `index_ident`.
    fn mk_mem_ele(
        &mut self,
        name            : &str,
        master_mem_blk_i: PyHcpIdent,
        index_ident     : PyHcpIdent,
        bit_width       : i32,
        is_read         : bool,
    ) -> PyHcpIdent {
        self.arena
            .make_mem_ele(true, name, master_mem_blk_i.into(), index_ident.into(), bit_width, is_read)
            .into()
    }

}
