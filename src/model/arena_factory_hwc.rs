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
    // Private helpers: register a freshly created HCP into the module on top
    // of module_trace_stack, routing by stage:
    //   CompInit / FlowBlockInit  → add directly to module
    //   FlowBlockBuild            → buffer in hcp_pending_buffer for later
    // -----------------------------------------------------------------------
    pub(super) fn reg_internal_hw_to_top_module(&mut self, i: HcpIdent) {
        let (module_i, stage) = self.peek_module_trace_stack();
        match stage {
            ModuleInitStage::CompInit | ModuleInitStage::FlowBlockInit => {
                let mut m = self.take_module(module_i);
                m.add_internal_hw(i);
                self.replace_back_module(module_i, m);
            }
            ModuleInitStage::FlowBlockBuild => {
                self.hcp_pending_buffer.push((i, false));
            }
        }
    }

    pub(super) fn reg_user_hw_to_top_module(&mut self, i: HcpIdent) {
        let (module_i, stage) = self.peek_module_trace_stack();
        match stage {
            ModuleInitStage::CompInit | ModuleInitStage::FlowBlockInit => {
                let mut m = self.take_module(module_i);
                m.add_user_hws(i);
                self.replace_back_module(module_i, m);
            }
            ModuleInitStage::FlowBlockBuild => {
                self.hcp_pending_buffer.push((i, true));
            }
        }
    }

    // -----------------------------------------------------------------------
    // HCP factories
    // -----------------------------------------------------------------------

    // ---- Reg ----------------------------------------------------------------

    pub fn make_reg(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_reg(Reg::new(false, name, bit_width));
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_reg(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_reg(Reg::new(true, name, bit_width));
        self.reg_user_hw_to_top_module(i);
        i
    }

    // ---- Wire ---------------------------------------------------------------

    pub fn make_wire(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_wire(Wire::new(false, name, bit_width));
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_wire(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_wire(Wire::new(true, name, bit_width));
        self.reg_user_hw_to_top_module(i);
        i
    }

    // ---- IoWire -------------------------------------------------------------

    pub fn make_io_wire(&mut self, name: &str, is_input: bool, src_i: HcpIdent) -> HcpIdent {
        let w = IoWire::new(false, name, is_input, src_i, self);
        let i = self.add_io_wire(w);
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_io_wire(&mut self, name: &str, is_input: bool, src_i: HcpIdent) -> HcpIdent {
        let w = IoWire::new(true, name, is_input, src_i, self);
        let i = self.add_io_wire(w);
        self.reg_user_hw_to_top_module(i);
        i
    }

    // ---- Val ----------------------------------------------------------------

    pub fn make_val(&mut self, name: &str, bit_width: i32, init_val: u64) -> HcpIdent {
        let i = self.add_val(Val::new(false, name, bit_width, init_val));
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_val(&mut self, name: &str, bit_width: i32, init_val: u64) -> HcpIdent {
        let i = self.add_val(Val::new(true, name, bit_width, init_val));
        self.reg_user_hw_to_top_module(i);
        i
    }

    // ---- MemEle -------------------------------------------------------------

    pub fn make_mem_ele(&mut self, name: &str, index_ident: HcpIdent, bit_width: i32, is_read: bool) -> HcpIdent {
        let i = self.add_mem_ele(MemEle::new(false, name, index_ident, bit_width, is_read));
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_mem_ele(&mut self, name: &str, index_ident: HcpIdent, bit_width: i32, is_read: bool) -> HcpIdent {
        let i = self.add_mem_ele(MemEle::new(true, name, index_ident, bit_width, is_read));
        self.reg_user_hw_to_top_module(i);
        i
    }

    // ---- MemBlk -------------------------------------------------------------

    pub fn make_mem_blk(&mut self, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        let i = self.add_mem_blk(MemBlk::new(false, name, bit_width, index_width));
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_mem_blk(&mut self, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        let i = self.add_mem_blk(MemBlk::new(true, name, bit_width, index_width));
        self.reg_user_hw_to_top_module(i);
        i
    }

    // ---- Expression ---------------------------------------------------------

    pub fn make_expression(&mut self, name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, a_slice: Option<Slice>, b_slice: Option<Slice>) -> HcpIdent {
        let a_slice = a_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&a)));
        let b_slice = b_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&b)));
        let i = self.add_expression(Expression::new(false, name, op, a, b, a_slice, b_slice, self));
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_expression(&mut self, name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, a_slice: Option<Slice>, b_slice: Option<Slice>) -> HcpIdent {
        let a_slice = a_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&a)));
        let b_slice = b_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&b)));
        let i = self.add_expression(Expression::new(true, name, op, a, b, a_slice, b_slice, self));
        self.reg_user_hw_to_top_module(i);
        i
    }
    pub fn make_expression_empty(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_expression(Expression::new_empty(false, name, bit_width));
        self.reg_internal_hw_to_top_module(i);
        i
    }
    pub fn mk_expression_empty(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let i = self.add_expression(Expression::new_empty(true, name, bit_width));
        self.reg_user_hw_to_top_module(i);
        i
    }
}
