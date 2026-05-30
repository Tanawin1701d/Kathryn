use std::fmt;

use crate::model::common::identifier::{IdentBase, Identifiable};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FlowBlockJoinPolicy {
    /// Regular child block nested inside a parent flow.
    SubFlow,
    /// Continuation branch of a conditional chain (elif / else / zelif / zelse).
    ConFlow,
    /// Block extracted and lowered to a single basic asm node.
    BasicNodeFlow,
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
        }
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
}

impl FlowBlockIdent {
    pub fn new(block_type: FlowBlockType, join_policy: FlowBlockJoinPolicy, name: &str) -> Self {
        Self {
            ident_base: IdentBase::new(false, name),
            block_type,
            join_policy,
        }
    }

    pub fn get_ident_base     (&self)     -> &IdentBase          { &self.ident_base }
    pub fn get_ident_base_mut (&mut self) -> &mut IdentBase      { &mut self.ident_base }
    pub fn get_block_type     (&self)     -> FlowBlockType        { self.block_type }
    pub fn get_join_policy    (&self)     -> FlowBlockJoinPolicy  { self.join_policy }

    pub fn build_unique_flow_block_name(&self) -> String {
        format!(
            "{}_{}_{}",
            self.block_type,
            self.ident_base.get_name(),
            self.ident_base.get_global_id()
        )
    }
}

impl Default for FlowBlockIdent {
    fn default() -> Self { Self::new(FlowBlockType::Sequential, FlowBlockJoinPolicy::SubFlow, "") }
}

impl Identifiable for FlowBlockIdent {
    fn get_ident_base(&self) -> &IdentBase { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }

    fn build_unique_name(&mut self) -> &str {
        let name = self.build_unique_flow_block_name();
        self.ident_base.set_name(&name);
        self.ident_base.get_name()
    }
}
