pub const LOGICAL_SIZE: usize = 1;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub enum LogicOp {
    // bitwise
    #[default]
    BitwiseAnd,  // &
    BitwiseOr,   // |
    BitwiseXor,  // ^
    BitwiseInvr, // ~
    BitwiseShl,  // <<
    BitwiseShr,  // >>
    // logical
    LogicalAnd,  // &&
    LogicalOr,   // ||
    LogicalNot,  // !
    // relational
    RelationEq,  // ==
    RelationNeq, // !=
    RelationLe,  // <
    RelationLeq, // <=
    RelationGe,  // >
    RelationGeq, // >=
    RelationSlt, // signed less than
    RelationSgt, // signed greater than
    // arithmetic
    ArithPlus,   // +
    ArithMinus,  // -
    ArithMul,    // *
    ArithDiv,    // /
    ArithDivr,   // %
    ExtendBit,   // bit extender
    // assign
    Assign,      // =
    // other
    Dummy,
}

impl LogicOp {
    pub fn to_op_str(self) -> &'static str {
        match self {
            LogicOp::BitwiseAnd  => "&",
            LogicOp::BitwiseOr   => "|",
            LogicOp::BitwiseXor  => "^",
            LogicOp::BitwiseInvr => "~",
            LogicOp::BitwiseShl  => "<<",
            LogicOp::BitwiseShr  => ">>",
            LogicOp::LogicalAnd  => "&&",
            LogicOp::LogicalOr   => "||",
            LogicOp::LogicalNot  => "!",
            LogicOp::RelationEq  => "==",
            LogicOp::RelationNeq => "!=",
            LogicOp::RelationLe  => "<",
            LogicOp::RelationLeq => "<=",
            LogicOp::RelationGe  => ">",
            LogicOp::RelationGeq => ">=",
            LogicOp::RelationSlt => "s<",
            LogicOp::RelationSgt => "s>",
            LogicOp::ArithPlus   => "+",
            LogicOp::ArithMinus  => "-",
            LogicOp::ArithMul    => "*",
            LogicOp::ArithDiv    => "/",
            LogicOp::ArithDivr   => "%",
            LogicOp::ExtendBit   => "ext",
            LogicOp::Assign      => "=",
            LogicOp::Dummy       => "DUMMYOP",
        }
    }

    pub fn is_single_opr(self) -> bool {
        matches!(self, LogicOp::Assign | LogicOp::BitwiseInvr | LogicOp::LogicalNot)
    }

    pub fn is_double_opr(self) -> bool {
        !self.is_single_opr()
    }
}

impl std::fmt::Display for LogicOp {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(self.to_op_str())
    }
}
