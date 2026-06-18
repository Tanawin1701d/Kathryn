use std::fmt;

use crate::model::common::identifier::{IdentBase, Identifiable};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FlowBlockJoinPolicy {
    /// Regular child block nested inside a parent flow.
    SubFlow,
    /// Continuation branch of a conditional chain (elif / else / zelif / zelse).
    ConFlow,
    /// Block extracted and lowered to a single basic asm node but use same entrance as SubFlow
    BasicNodeFlow,
}

/// Tracking state of a flow block while it sits on the init stack.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum BlockTrackStatus {
    /// Block is active and still accepts new sub-blocks / nodes.
    OpenForSubBlock,
    /// Block has been logically closed but is kept on the stack so a following
    /// chain branch (elif / else / zelif / zelse) can still attach to it.
    LazyClosed,
    /// Block is fully closed — no further attachment is allowed.
    FullyClosed,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FlowBlockType {
    Sequential,
    Parallel,
    CondIf,
    CondElif,
    ZeroCondIf,
    ZeroCondElif,
    ZeroSwitch,
    ZeroSwitchCase,
    WhileLoop,
    DoWhile,
    CounterLoop,
    Wait,
    Pipeline,
}

impl FlowBlockType {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Sequential     => "SEQUENTIAL",
            Self::Parallel       => "PARALLEL",
            Self::CondIf         => "COND_IF",
            Self::CondElif       => "COND_ELIF",
            Self::ZeroCondIf     => "ZERO_COND_IF",
            Self::ZeroCondElif   => "ZERO_COND_ELIF",
            Self::ZeroSwitch     => "ZERO_SWITCH",
            Self::ZeroSwitchCase => "ZERO_SWITCH_CASE",
            Self::WhileLoop      => "WHILE_LOOP",
            Self::DoWhile        => "DO_WHILE",
            Self::CounterLoop    => "COUNTER_LOOP",
            Self::Wait           => "WAIT",
            Self::Pipeline       => "PIPELINE",
        }
    }

    /// Stable variant name (PascalCase) — backs the Python `FlowBlockType`
    /// IntEnum. Exhaustive, so a new variant fails to compile until named here.
    pub fn variant_name(self) -> &'static str {
        match self {
            Self::Sequential     => "Sequential",
            Self::Parallel       => "Parallel",
            Self::CondIf         => "CondIf",
            Self::CondElif       => "CondElif",
            Self::ZeroCondIf     => "ZeroCondIf",
            Self::ZeroCondElif   => "ZeroCondElif",
            Self::ZeroSwitch     => "ZeroSwitch",
            Self::ZeroSwitchCase => "ZeroSwitchCase",
            Self::WhileLoop      => "WhileLoop",
            Self::DoWhile        => "DoWhile",
            Self::CounterLoop    => "CounterLoop",
            Self::Wait           => "Wait",
            Self::Pipeline       => "Pipeline",
        }
    }

    /// Variant → 0-based declaration-order index (inverse of `from_index`).
    pub fn to_index(self) -> u32 {
        match self {
            Self::Sequential     => 0,
            Self::Parallel       => 1,
            Self::CondIf         => 2,
            Self::CondElif       => 3,
            Self::ZeroCondIf     => 4,
            Self::ZeroCondElif   => 5,
            Self::ZeroSwitch     => 6,
            Self::ZeroSwitchCase => 7,
            Self::WhileLoop      => 8,
            Self::DoWhile        => 9,
            Self::CounterLoop    => 10,
            Self::Wait           => 11,
            Self::Pipeline       => 12,
        }
    }

    /// Map a 0-based index (variant declaration order) to its `FlowBlockType`.
    /// `None` only when `idx` is past the last variant.
    pub fn from_index(idx: u32) -> Option<FlowBlockType> {
        let t = match idx {
            0  => Self::Sequential,
            1  => Self::Parallel,
            2  => Self::CondIf,
            3  => Self::CondElif,
            4  => Self::ZeroCondIf,
            5  => Self::ZeroCondElif,
            6  => Self::ZeroSwitch,
            7  => Self::ZeroSwitchCase,
            8  => Self::WhileLoop,
            9  => Self::DoWhile,
            10 => Self::CounterLoop,
            11 => Self::Wait,
            12 => Self::Pipeline,
            _  => return None,
        };
        Some(t)
    }
}

impl fmt::Display for FlowBlockType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.as_str())
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct FlowBlockIdent {
    ident_base  : IdentBase,
    block_type  : FlowBlockType,
    join_policy : FlowBlockJoinPolicy,
    chain_master: bool,
}

impl FlowBlockIdent {
    pub fn new(block_type: FlowBlockType, join_policy: FlowBlockJoinPolicy, name: &str, chain_master: bool) -> Self {
        let mut s = Self {
            ident_base: IdentBase::new(false),
            block_type,
            join_policy,
            chain_master,
        };
        s.ident_base.set_rel_name(name);
        let abs = s.build_unique_flow_block_name();
        s.ident_base.set_abs_name(&abs);
        s
    }

    pub fn get_ident_base     (&self)     -> &IdentBase          { &self.ident_base }
    pub fn get_ident_base_mut (&mut self) -> &mut IdentBase      { &mut self.ident_base }
    pub fn get_block_type     (&self)     -> FlowBlockType        { self.block_type }
    pub fn get_join_policy    (&self)     -> FlowBlockJoinPolicy  { self.join_policy }
    pub fn get_chain_master   (&self)     -> bool                 { self.chain_master }

    pub fn build_unique_flow_block_name(&self) -> String {
        format!(
            "{}_{}_{}",
            self.block_type,
            self.ident_base.get_rel_name(),
            self.ident_base.get_global_id()
        )
    }
}

impl Default for FlowBlockIdent {
    fn default() -> Self { Self::new(FlowBlockType::Sequential, FlowBlockJoinPolicy::SubFlow, "", false) }
}

impl Identifiable for FlowBlockIdent {
    fn get_ident_base(&self) -> &IdentBase { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
}
