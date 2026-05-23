use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::{reg_width, slice_to_verilog};
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::expression::Expression;
use crate::model::model_arena::ModelArena;

impl HcpBaseVb for Expression {
    fn gen_type         (&self) -> String { format!("wire {}", reg_width(self.get_des_slice().get_size())) }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String {
        format!("{} {};", self.gen_type(), self.gen_var_name())
    }

    fn gen_procedure_blk(&self, _idx: u32, _arena: &mut ModelArena) -> String {
        if !self.is_value_assigned() { return String::new(); }

        let a_i     = match self.get_operand_a() { Some(x) => x, None => return String::new() };
        let a_slice = self.get_operand_a_slice().unwrap_or_default();
        let a_str   = format!("{}{}", a_i.get_global_name(), slice_to_verilog(&a_slice));

        let rhs = if self.get_op() == LogicOp::Assign {
            // simple wire-through — no operator
            a_str
        } else if self.get_op().is_single_opr() {
            // prefix unary: ~a, !a
            format!("{}{}", self.get_op().to_op_str(), a_str)
        } else {
            // binary: a OP b
            let b_i     = match self.get_operand_b() { Some(x) => x, None => return String::new() };
            let b_slice = self.get_operand_b_slice().unwrap_or_default();
            let b_str   = format!("{}{}", b_i.get_global_name(), slice_to_verilog(&b_slice));
            format!("{} {} {}", a_str, self.get_op().to_op_str(), b_str)
        };

        format!("assign {} = {};\n", self.gen_var_name(), rhs)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_expression(*self); }
}
