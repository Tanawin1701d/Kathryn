use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::expression::Expression;
use crate::model::hw_component::memBlk::MemBlk;
use crate::model::hw_component::memEle::MemEle;
use crate::model::hw_component::reg::Reg;
use crate::model::hw_component::val::Val;
use crate::model::hw_component::wire::Wire;
use crate::model::hw_component::io_wire::IoWire;
use crate::model::model_arena::{ModelArena, ModuleInitStage};

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    // -----------------------------------------------------------------------
    // Private helper: register a freshly created HCP into the module on top
    // of module_trace_stack, routing by stage:
    //   CompInit / FlowBlockInit  → add directly to module
    //   FlowBlockBuild            → buffer in hcp_pending_buffer for later
    // Also stamps master_module_i on the ident regardless of stage.
    // -----------------------------------------------------------------------
    pub(super) fn stamp_hw_to_parent_module(&mut self, mut i: HcpIdent, is_user: bool) -> HcpIdent {
        let (module_i, stage) = self.peek_module_trace_stack();
        i.set_master_module_i(module_i);
        *self.get_hcp_ident_mut(&i).get_ident_mut() = i;
        match stage {
            ModuleInitStage::CompInit | ModuleInitStage::FlowBlockInit => {
                let mut m = self.take_module(module_i);
                if is_user { m.add_user_hws(i); } else { m.add_internal_hw(i); }
                self.replace_back_module(module_i, m);
            }
            ModuleInitStage::FlowBlockBuild => {
                self.hcp_pending_buffer.push((i, is_user));
            }
        }
        i
    }

    // -----------------------------------------------------------------------
    // HCP factories
    // -----------------------------------------------------------------------

    // ---- Reg ----------------------------------------------------------------

    pub fn make_reg(&mut self, is_user_com: bool, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_reg(Reg::new(is_user_com, name, bit_width));
        self.stamp_hw_to_parent_module(i, is_user_com)
    }

    // ---- Wire ---------------------------------------------------------------

    pub fn make_wire(&mut self, is_user_com: bool, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_wire(Wire::new(is_user_com, name, bit_width));
        self.stamp_hw_to_parent_module(i, is_user_com)
    }

    // ---- IoWire -------------------------------------------------------------

    pub fn make_io_wire(&mut self, is_user_com: bool, name: &str, is_input: bool, actual_src_i: HcpIdent, agent_src_i: HcpIdent) -> HcpIdent {
        let w = IoWire::new(is_user_com, name, is_input, actual_src_i, agent_src_i, self);
        let i = self.add_io_wire(w);
        self.stamp_hw_to_parent_module(i, is_user_com)
    }

    // ---- Val ----------------------------------------------------------------

    pub fn make_val(&mut self, is_user_com: bool, name: &str, bit_width: i32, init_val: u64) -> HcpIdent {
        let i = self.add_val(Val::new(is_user_com, name, bit_width, init_val));
        self.stamp_hw_to_parent_module(i, is_user_com)
    }

    // ---- MemEle -------------------------------------------------------------

    pub fn make_mem_ele(&mut self, is_user_com: bool, name: &str, index_ident: HcpIdent, bit_width: i32, is_read: bool) -> HcpIdent {
        let i = self.add_mem_ele(MemEle::new(is_user_com, name, index_ident, bit_width, is_read));
        self.stamp_hw_to_parent_module(i, is_user_com)
    }

    // ---- MemBlk -------------------------------------------------------------

    pub fn make_mem_blk(&mut self, is_user_com: bool, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        let i = self.add_mem_blk(MemBlk::new(is_user_com, name, bit_width, index_width));
        self.stamp_hw_to_parent_module(i, is_user_com)
    }

    // ---- Expression ---------------------------------------------------------

    pub fn make_expression(&mut self, is_user_com: bool, name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, a_slice: Option<Slice>, b_slice: Option<Slice>) -> HcpIdent {
        let a_slice = a_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&a)));
        let b_slice = b_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&b)));
        let i = self.add_expression(Expression::new(is_user_com, name, op, a, b, a_slice, b_slice, self));
        self.stamp_hw_to_parent_module(i, is_user_com)
    }
    pub fn make_expression_empty(&mut self, is_user_com: bool, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_expression(Expression::new_empty(is_user_com, name, bit_width));
        self.stamp_hw_to_parent_module(i, is_user_com)
    }
}
