use std::collections::{HashMap, HashSet};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::model::common::identifier::{IdentBase, Identifiable};

// Combinational expression node: computes `op(operand_a, operand_b)` and exposes the result
// as a signal.  Relational and logical ops always produce a 1-bit output; arithmetic ops
// inherit their width from operand_a.  When operand_b is a compile-time constant, use
// `operand_c` instead — it holds the raw u64 value and `operand_b` stays None.
#[derive(Default)]
pub struct Expression {
    assign         : HcpAssign,
    ident          : HcpIdent,
    bit_width      : i32,              // output width; 1 for relational/logical ops
    op             : LogicOp,
    operand_a      : Option<HcpIdent>, // None until assign_operand is called on empty exprs
    operand_b      : Option<HcpIdent>, // None when operand_c is used instead
    operand_a_slice: Option<Slice>,
    operand_b_slice: Option<Slice>,
    operand_c      : Option<i32>,      // constant literal for b; mutually exclusive with operand_b
    value_assigned : bool,             // guard against double-assign on empty expressions
}

impl Expression {
    // ---- constructors ----

    /// Expression with a fully defined operation and both operands.
    pub fn new(is_user_com: bool,
               name       : &str,
               op         : LogicOp,
               a          : HcpIdent,
               b          : HcpIdent,
               a_slice    : Slice,
               b_slice    : Slice) -> Self {

        let bit_width = match op {
            // result 1 bit length
            LogicOp::LogicalAnd
            | LogicOp::LogicalOr
            | LogicOp::RelationEq
            | LogicOp::RelationNeq
            | LogicOp::RelationLe
            | LogicOp::RelationLeq
            | LogicOp::RelationGe
            | LogicOp::RelationGeq
            | LogicOp::RelationSlt
            | LogicOp::RelationSgt => 1,
            // result is same width as a_slice
            LogicOp::BitwiseAnd
            | LogicOp::BitwiseOr
            | LogicOp::BitwiseXor
            | LogicOp::BitwiseShl
            | LogicOp::BitwiseShr
            | LogicOp::ArithPlus
            | LogicOp::ArithMinus
            | LogicOp::ArithMul
            | LogicOp::ArithDiv
            | LogicOp::ArithDivr => a_slice.get_size(),
            // other not support for this constructor
            _ => panic!("Expression::new — unsupported op {:?} for this constructor", op),
        };

        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::new(HwComponentType::Expression, HcpSensitiveType::ReadOnly, is_user_com, name),
            bit_width,
            op,
            operand_a      : Some(a),
            operand_b      : Some(b),
            operand_a_slice: Some(a_slice),
            operand_b_slice: Some(b_slice),
            operand_c      : None,
            value_assigned : true,
        }
    }



    /// Expression with a single operand (unary ops: `BitwiseInvr`, `LogicalNot`, `SliceBit`).
    /// `operand_b` and `operand_c` stay None.
    pub fn new_single_operand(is_user_com: bool,
                              name       : &str,
                              op         : LogicOp,
                              a          : HcpIdent,
                              a_slice    : Slice) -> Self {
        let bit_width = match op {
            // result 1 bit length
            LogicOp::LogicalNot  => 1,
            // result is same width as a_slice
            LogicOp::BitwiseInvr => a_slice.get_size(),
            LogicOp::SliceBit    => a_slice.get_size(),
            // other not support for this constructor
            _ => panic!("Expression::new_single_operand — unsupported op {:?} for this constructor", op),
        };
        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::new(HwComponentType::Expression, HcpSensitiveType::ReadOnly, is_user_com, name),
            bit_width,
            op,
            operand_a      : Some(a),
            operand_b      : None,
            operand_a_slice: Some(a_slice),
            operand_b_slice: None,
            operand_c      : None,
            value_assigned : true,
        }
    }

    /// Expression where the right-hand operand is a constant u64 rather than a signal.
    /// `operand_b` stays None; `operand_c` holds the literal value.
    pub fn new_with_const(is_user_com : bool,
                          name        : &str,
                          op          : LogicOp,
                          a           : HcpIdent,
                          c           : i32,
                          a_slice     : Slice) -> Self {
        let bit_width = match op {
            LogicOp::ExtendBit => c,
            _ => panic!("Expression::new_with_const — unsupported op {:?} for this constructor", op),
        };
        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::new(HwComponentType::Expression, HcpSensitiveType::ReadOnly, is_user_com, name),
            bit_width,
            op,
            operand_a      : Some(a),
            operand_b      : None,
            operand_a_slice: Some(a_slice),
            operand_b_slice: None,
            operand_c      : Some(c),
            value_assigned : true,
        }
    }

    /// Unassigned expression — operand will be supplied later via `assign_operand`.
    pub fn new_empty(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::new(HwComponentType::Expression, HcpSensitiveType::ReadOnly, is_user_com, name),
            bit_width,
            op             : LogicOp::Assign,
            operand_a      : None,
            operand_b      : None,
            operand_a_slice: None,
            operand_b_slice: None,
            operand_c      : None,
            value_assigned : false,
        }
    }

    // ---- accessors ----

    pub fn get_ident          (&    self) ->      HcpIdent {      self.ident }
    pub fn get_ident_mut      (&mut self) -> &mut HcpIdent { &mut self.ident }

    pub fn get_op             (&    self) -> LogicOp          { self.op }
    pub fn get_operand_a      (&    self) -> Option<HcpIdent> { self.operand_a }
    pub fn get_operand_b      (&    self) -> Option<HcpIdent> { self.operand_b }
    pub fn get_operand_a_slice(&    self) -> Option<Slice>    { self.operand_a_slice }
    pub fn get_operand_b_slice(&    self) -> Option<Slice>    { self.operand_b_slice }
    pub fn get_operand_c      (&    self) -> Option<i32>      { self.operand_c }
    pub fn is_value_assigned  (&    self) -> bool             { self.value_assigned }

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
              srci     : HcpIdent,
              des_slice: Option<Slice>,
              src_slice: Slice,
              arena    : &mut ModelArena) -> NcpIdent {
        panic!("Expression::do_asm — not implemented")
    }
}

// Expression overrides gather/remap to walk operand handles instead of the UE pool.

impl Identifiable for Expression {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for Expression {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Expression {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_expression(*self); }

    /// Expressions depend on their operands directly, not on UEs in the update pool.
    fn gather_dep_hcps(&self, _arena: &mut ModelArena, out: &mut HashSet<HcpIdent>) {
        if let Some(a_i) = self.operand_a { out.insert(a_i); }
        if let Some(b_i) = self.operand_b { out.insert(b_i); }
    }

    /// Remap operand handles according to `map`.
    fn remap_dep_hcps(&mut self, map: &HashMap<HcpIdent, HcpIdent>, _arena: &mut ModelArena) {
        for opt in [&mut self.operand_a, &mut self.operand_b] {
            if let Some(des_opr_i) = opt {
                if let Some(&src_opr_i) = map.get(des_opr_i) {
                    *des_opr_i = src_opr_i;
                }
            }
        }
    }
}
