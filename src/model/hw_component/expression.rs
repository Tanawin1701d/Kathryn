use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::common::identifier::{IdentBase, Identifiable};

pub struct Expression {
    assign        : HcpAssign,
    ident         : HcpIdent,
    bit_width     : i32,
    op            : LogicOp,
    operand_a     : Option<HcpIdent>,
    operand_b     : Option<HcpIdent>,
    value_assigned: bool,
}

impl Expression {
    /// Expression with a fully defined operation and both operands.
    pub fn new(is_user_com: bool,
               name       : &str,
               op         : LogicOp,
               a          : HcpIdent,
               b          : HcpIdent,
               model_arena: &ModelArena) -> Self {
        
        let bit_width = model_arena.get_hw_bit_sz(&a);
        
        Self {
            assign        : HcpAssign::new(),
            ident         : HcpIdent::new(HwComponentType::Expression, is_user_com, name),
            bit_width,
            op,
            operand_a     : Some(a),
            operand_b     : Some(b),
            value_assigned: true,
        }
    }

    /// Unassigned expression — operand will be supplied later via `assign_operand`.
    pub fn new_empty(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign        : HcpAssign::new(),
            ident         : HcpIdent::new(HwComponentType::Expression, is_user_com, name),
            bit_width,
            op            : LogicOp::Assign,
            operand_a     : None,
            operand_b     : None,
            value_assigned: false,
        }
    }

    pub fn mk(name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, model_arena: &ModelArena) -> Self {
        Expression::new(true, name, op, a, b, model_arena)
    }

    pub fn mk_empty(name: &str, bit_width: i32) -> Self {
        Expression::new_empty(true, name, bit_width)
    }

    pub fn get_ident(&self) -> HcpIdent { self.ident }

    pub fn get_op           (&self) -> LogicOp           { self.op }
    pub fn get_operand_a    (&self) -> Option<HcpIdent> { self.operand_a }
    pub fn get_operand_b    (&self) -> Option<HcpIdent> { self.operand_b }
    pub fn is_value_assigned(&self) -> bool             { self.value_assigned }

    /// Provide the source operand for an unassigned expression (non-block assign).
    /// Panics if the expression was already assigned or the slice sizes don't match.
    pub fn assign_operand(&mut self, src: HcpIdent, src_slice: Slice) {
        assert!(!self.value_assigned, "multiple expression assign detected");
        assert_eq!(
            src_slice.get_size(), self.bit_width,
            "src expression assign wrapper doesn't cover entire expression"
        );
        self.operand_a     = Some(src);
        self.value_assigned = true;
    }
}

impl HcpReadable for Expression {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for Expression {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              arena    : &mut ModelArena) -> AssignMeta {
        self.gen_asm_meta(srci, des_slice, src_slice, arena)
    }
}

impl HcpAccessible for Expression {
    fn get_bit_width(&self) -> usize { self.bit_width as usize }
}

impl Identifiable for Expression {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
