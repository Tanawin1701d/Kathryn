// Python-facing HWC arena operations. Mirrors the host `arena_impl_hwc.rs`
// higher-level helpers (those beyond plain CRUD) that Python needs to drive
// model construction.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::hw_component::common::slice_py::PySlice;
use super::hw_component::common::operand_py::PyOperand;
use crate::model::controller::clock_mode::get_global_clk_mode;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::slice::Slice;

#[pymethods]
impl PyModelArena {
    // Build a basic assignment node `des_i <= src_i` and attach it to the active
    // flow block (or the top module if none is building). Slices optional
    // (default = full signal); returns nothing, mirroring the host signature.
    #[pyo3(signature = (des_i, src, src_slice=None, des_slice=None))]
    fn gen_basic_assign(
        &mut self,
        des_i    : PyHcpIdent,
        src      : PyOperand,
        src_slice: Option<PySlice>,
        des_slice: Option<PySlice>,
    ) {
        // An int source is wrapped into a val sized to the destination's width;
        // its source slice is that val's full range (the size-match assert in the
        // host runs before any default-slice resolution, so it must be concrete).
        let (src_i, src_slice) = match src {
            PyOperand::Ident(x) => (x.into(), src_slice.map(Into::into).unwrap_or_default()),
            PyOperand::Int(n)   => {
                let w = self.arena.get_hw_bit_sz(&des_i.into());
                (self.make_const_val("const", &n, w), Slice::new(0, w))
            }
        };
        self.arena.gen_basic_assign(
            des_i.into(),
            src_i,
            des_slice.map(Into::into),
            src_slice,
        );
    }

    // Full bit-width of the component, used to seed a SignalRef's full slice.
    // A MemBlk has no scalar slice (its HcpAssignable methods panic), so read its
    // per-word width directly; every other type goes through the arena slice size.
    fn get_hw_bit_sz(&mut self, hcp_i: PyHcpIdent) -> i32 {
        let ident: HcpIdent = hcp_i.into();
        self.arena.get_hw_bit_sz(&ident)
    }

    // Record a reg's reset value; the reset event clocks off the global clk mode
    // (the reg's own sensitivity) and is built during the host build pass.
    fn set_reg_reset(&mut self, reg_i: PyHcpIdent, reset_val_i: PyHcpIdent) {
        self.arena.set_reg_reset(reg_i.into(), reset_val_i.into(), get_global_clk_mode());
    }

    // Record a wire's combinational fallback value (internal-low priority default event).
    fn set_wire_default(&mut self, wire_i: PyHcpIdent, default_val_i: PyHcpIdent) {
        self.arena.set_wire_default(wire_i.into(), default_val_i.into());
    }

    // Stamp an HCP as an IO port (direction + user-facing name).
    fn mark_as_io(&mut self, hcp_i: PyHcpIdent, is_input: bool, io_name: String) {
        self.arena.mark_as_io(hcp_i.into(), is_input, io_name);
    }

    // True if the HCP has been marked as an IO port.
    fn is_marked_as_io(&self, hcp_i: PyHcpIdent) -> bool {
        self.arena.is_marked_as_io(&hcp_i.into())
    }
}
