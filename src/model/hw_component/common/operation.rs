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
    ArithShrA,   // >>> arithmetic (sign-preserving) shift right
    ArithDivS,   // signed /
    ArithRemS,   // signed %
    ExtendBit,   // bit extender
    SliceBit,    // a[slice] bit/slice selector (single operand)
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
            LogicOp::ArithShrA   => ">>>",
            LogicOp::ArithDivS   => "s/",
            LogicOp::ArithRemS   => "s%",
            LogicOp::ExtendBit   => "ext",
            LogicOp::SliceBit    => "slice",
            LogicOp::Assign      => "=",
            LogicOp::Dummy       => "DUMMYOP",
        }
    }

    /// Stable variant name (used to surface ops to other languages). Exhaustive,
    /// so a new `LogicOp` variant fails to compile until it is named here.
    pub fn variant_name(self) -> &'static str {
        match self {
            LogicOp::BitwiseAnd  => "BitwiseAnd",
            LogicOp::BitwiseOr   => "BitwiseOr",
            LogicOp::BitwiseXor  => "BitwiseXor",
            LogicOp::BitwiseInvr => "BitwiseInvr",
            LogicOp::BitwiseShl  => "BitwiseShl",
            LogicOp::BitwiseShr  => "BitwiseShr",
            LogicOp::LogicalAnd  => "LogicalAnd",
            LogicOp::LogicalOr   => "LogicalOr",
            LogicOp::LogicalNot  => "LogicalNot",
            LogicOp::RelationEq  => "RelationEq",
            LogicOp::RelationNeq => "RelationNeq",
            LogicOp::RelationLe  => "RelationLe",
            LogicOp::RelationLeq => "RelationLeq",
            LogicOp::RelationGe  => "RelationGe",
            LogicOp::RelationGeq => "RelationGeq",
            LogicOp::RelationSlt => "RelationSlt",
            LogicOp::RelationSgt => "RelationSgt",
            LogicOp::ArithPlus   => "ArithPlus",
            LogicOp::ArithMinus  => "ArithMinus",
            LogicOp::ArithMul    => "ArithMul",
            LogicOp::ArithDiv    => "ArithDiv",
            LogicOp::ArithDivr   => "ArithDivr",
            LogicOp::ArithShrA   => "ArithShrA",
            LogicOp::ArithDivS   => "ArithDivS",
            LogicOp::ArithRemS   => "ArithRemS",
            LogicOp::ExtendBit   => "ExtendBit",
            LogicOp::SliceBit    => "SliceBit",
            LogicOp::Assign      => "Assign",
            LogicOp::Dummy       => "Dummy",
        }
    }

    /// Map a 0-based index (variant declaration order) to its `LogicOp` — the
    /// inverse of the variant order. `None` only when `idx` is past the last
    /// variant.
    pub fn from_index(idx: u32) -> Option<LogicOp> {
        let op = match idx {
            0  => LogicOp::BitwiseAnd,
            1  => LogicOp::BitwiseOr,
            2  => LogicOp::BitwiseXor,
            3  => LogicOp::BitwiseInvr,
            4  => LogicOp::BitwiseShl,
            5  => LogicOp::BitwiseShr,
            6  => LogicOp::LogicalAnd,
            7  => LogicOp::LogicalOr,
            8  => LogicOp::LogicalNot,
            9  => LogicOp::RelationEq,
            10 => LogicOp::RelationNeq,
            11 => LogicOp::RelationLe,
            12 => LogicOp::RelationLeq,
            13 => LogicOp::RelationGe,
            14 => LogicOp::RelationGeq,
            15 => LogicOp::RelationSlt,
            16 => LogicOp::RelationSgt,
            17 => LogicOp::ArithPlus,
            18 => LogicOp::ArithMinus,
            19 => LogicOp::ArithMul,
            20 => LogicOp::ArithDiv,
            21 => LogicOp::ArithDivr,
            22 => LogicOp::ArithShrA,
            23 => LogicOp::ArithDivS,
            24 => LogicOp::ArithRemS,
            25 => LogicOp::ExtendBit,
            26 => LogicOp::SliceBit,
            27 => LogicOp::Assign,
            28 => LogicOp::Dummy,
            _  => return None,
        };
        Some(op)
    }

    pub fn is_single_opr(self) -> bool {
        matches!(self, LogicOp::Assign | LogicOp::BitwiseInvr | LogicOp::LogicalNot | LogicOp::SliceBit)
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
