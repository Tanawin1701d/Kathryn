use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::common::identifier::{IdentBase, Identifiable};

#[derive(Default)]
pub struct Expression {
    assign        : HcpAssign,
    ident         : HcpIdent,
    bit_width     : i32,
    op            : LogicOp,
    operand_a     : Option<HcpIdent>,
    operand_b     : Option<HcpIdent>,
    operand_a_slice: Option<Slice>,
    operand_b_slice: Option<Slice>,
    value_assigned: bool,
}

impl Expression {
    /// Expression with a fully defined operation and both operands.
    pub fn new(is_user_com: bool,
               name       : &str,
               op         : LogicOp,
               a          : HcpIdent,
               b          : HcpIdent,
               a_slice    : Slice,
               b_slice    : Slice,
               model_arena: &ModelArena) -> Self {

        let bit_width = match op {
            LogicOp::LogicalAnd
            | LogicOp::LogicalOr
            | LogicOp::LogicalNot
            | LogicOp::RelationEq
            | LogicOp::RelationNeq
            | LogicOp::RelationLe
            | LogicOp::RelationLeq
            | LogicOp::RelationGe
            | LogicOp::RelationGeq
            | LogicOp::RelationSlt
            | LogicOp::RelationSgt => 1,
            _ => model_arena.get_hw_bit_sz(&a),
        };

        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::new(HwComponentType::Expression, is_user_com, name),
            bit_width,
            op,
            operand_a      : Some(a),
            operand_b      : Some(b),
            operand_a_slice: Some(a_slice),
            operand_b_slice: Some(b_slice),
            value_assigned : true,
        }
    }

    /// Unassigned expression — operand will be supplied later via `assign_operand`.
    pub fn new_empty(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::new(HwComponentType::Expression, is_user_com, name),
            bit_width,
            op             : LogicOp::Assign,
            operand_a      : None,
            operand_b      : None,
            operand_a_slice: None,
            operand_b_slice: None,
            value_assigned : false,
        }
    }

    pub fn mk(name: &str, op: LogicOp, a: HcpIdent, b: HcpIdent, a_slice: Slice, b_slice: Slice, model_arena: &ModelArena) -> Self {
        Expression::new(true, name, op, a, b, a_slice, b_slice, model_arena)
    }

    pub fn mk_empty(name: &str, bit_width: i32) -> Self {
        Expression::new_empty(true, name, bit_width)
    }

    pub fn get_ident(&self) -> HcpIdent { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }

    pub fn get_op             (&self) -> LogicOp         { self.op }
    pub fn get_operand_a      (&self) -> Option<HcpIdent> { self.operand_a }
    pub fn get_operand_b      (&self) -> Option<HcpIdent> { self.operand_b }
    pub fn get_operand_a_slice(&self) -> Option<Slice>    { self.operand_a_slice }
    pub fn get_operand_b_slice(&self) -> Option<Slice>    { self.operand_b_slice }
    pub fn is_value_assigned  (&self) -> bool             { self.value_assigned }

    /// Provide the source operand for an unassigned expression (non-block assign).
    /// Panics if the expression was already assigned or the slice sizes don't match.
    pub fn assign_operand(&mut self, src: HcpIdent, src_slice: Slice) {
        assert!(!self.value_assigned, "multiple expression assign detected");
        assert_eq!(
            src_slice.get_size(), self.bit_width,
            "src expression assign wrapper doesn't cover entire expression"
        );
        self.operand_a       = Some(src);
        self.operand_a_slice = Some(src_slice);
        self.value_assigned  = true;
    }
}

impl HcpAssignable for Expression {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              arena    : &mut ModelArena) -> AssignMeta {
        self.gen_asm_meta(self.ident, srci, des_slice, src_slice, arena)
    }
}

impl Identifiable for Expression {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for Expression {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Expression {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_expression(*self); }
}
