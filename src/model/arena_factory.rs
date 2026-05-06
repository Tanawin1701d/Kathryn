use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::expression::Expression;
use crate::model::hw_component::memBlk::MemBlk;
use crate::model::hw_component::memEle::MemEle;
use crate::model::hw_component::reg::Reg;
use crate::model::hw_component::val::Val;
use crate::model::hw_component::wire::Wire;
use crate::model::model_arena::ModelArena;

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    pub fn make_reg(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        self.add_reg(Reg::new(false, name, bit_width))
    }
    pub fn mk_reg(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        self.add_reg(Reg::new(true, name, bit_width))
    }

    pub fn make_wire(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        self.add_wire(Wire::new(false, name, bit_width))
    }
    pub fn mk_wire(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        self.add_wire(Wire::new(true, name, bit_width))
    }

    pub fn make_val(&mut self, name: &str, bit_width: i32, init_val: u64) -> HcpIdent {
        self.add_val(Val::new(false, name, bit_width, init_val))
    }
    pub fn mk_val(&mut self, name: &str, bit_width: i32, init_val: u64) -> HcpIdent {
        self.add_val(Val::new(true, name, bit_width, init_val))
    }

    pub fn make_mem_ele(&mut self, name: &str, index_ident: HcpIdent, bit_width: i32, is_read: bool) -> HcpIdent {
        self.add_mem_ele(MemEle::new(false, name, index_ident, bit_width, is_read))
    }
    pub fn mk_mem_ele(&mut self, name: &str, index_ident: HcpIdent, bit_width: i32, is_read: bool) -> HcpIdent {
        self.add_mem_ele(MemEle::new(true, name, index_ident, bit_width, is_read))
    }

    pub fn make_mem_blk(&mut self, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        self.add_mem_blk(MemBlk::new(false, name, bit_width, index_width))
    }
    pub fn mk_mem_blk(&mut self, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        self.add_mem_blk(MemBlk::new(true, name, bit_width, index_width))
    }

    pub fn make_expression(&mut self, name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, a_slice: Option<Slice>, b_slice: Option<Slice>) -> HcpIdent {
        let a_slice = a_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&a)));
        let b_slice = b_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&b)));
        self.add_expression(Expression::new(false, name, op, a, b, a_slice, b_slice, self))
    }
    pub fn mk_expression(&mut self, name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, a_slice: Option<Slice>, b_slice: Option<Slice>) -> HcpIdent {
        let a_slice = a_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&a)));
        let b_slice = b_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&b)));
        self.add_expression(Expression::new(true, name, op, a, b, a_slice, b_slice, self))
    }
    pub fn make_expression_empty(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        self.add_expression(Expression::new_empty(false, name, bit_width))
    }
    pub fn mk_expression_empty(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        self.add_expression(Expression::new_empty(true, name, bit_width))
    }
}
