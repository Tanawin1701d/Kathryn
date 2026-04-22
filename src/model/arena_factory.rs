use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
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
        let r = Reg::new(false, name, bit_width); let i = r.get_ident(); self.add_reg(r); i
    }
    pub fn mk_reg(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let r = Reg::new(true, name, bit_width); let i = r.get_ident(); self.add_reg(r); i
    }

    pub fn make_wire(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let w = Wire::new(false, name, bit_width); let i = w.get_ident(); self.add_wire(w); i
    }
    pub fn mk_wire(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let w = Wire::new(true, name, bit_width); let i = w.get_ident(); self.add_wire(w); i
    }

    pub fn make_val(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let v = Val::new(false, name, bit_width); let i = v.get_ident(); self.add_val(v); i
    }
    pub fn mk_val(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let v = Val::new(true, name, bit_width); let i = v.get_ident(); self.add_val(v); i
    }

    pub fn make_mem_ele(&mut self, name: &str, index_ident: HcpIdent, bit_width: i32, is_read: bool) -> HcpIdent {
        let e = MemEle::new(false, name, index_ident, bit_width, is_read); let i = e.get_ident(); self.add_mem_ele(e); i
    }
    pub fn mk_mem_ele(&mut self, name: &str, index_ident: HcpIdent, bit_width: i32, is_read: bool) -> HcpIdent {
        let e = MemEle::new(true, name, index_ident, bit_width, is_read); let i = e.get_ident(); self.add_mem_ele(e); i
    }

    pub fn make_mem_blk(&mut self, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        let b = MemBlk::new(false, name, bit_width, index_width); let i = b.get_ident(); self.add_mem_blk(b); i
    }
    pub fn mk_mem_blk(&mut self, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        let b = MemBlk::new(true, name, bit_width, index_width); let i = b.get_ident(); self.add_mem_blk(b); i
    }

    pub fn make_expression(&mut self, name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, bit_width: i32) -> HcpIdent {
        let e = Expression::new(false, name, op, a, b, bit_width); let i = e.get_ident(); self.add_expression(e); i
    }
    pub fn mk_expression(&mut self, name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, bit_width: i32) -> HcpIdent {
        let e = Expression::new(true, name, op, a, b, bit_width); let i = e.get_ident(); self.add_expression(e); i
    }
    pub fn make_expression_empty(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let e = Expression::new_empty(false, name, bit_width); let i = e.get_ident(); self.add_expression(e); i
    }
    pub fn mk_expression_empty(&mut self, name: &str, bit_width: i32) -> HcpIdent {
        let e = Expression::new_empty(true, name, bit_width); let i = e.get_ident(); self.add_expression(e); i
    }
}
