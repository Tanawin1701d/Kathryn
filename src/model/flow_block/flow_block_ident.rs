use std::fmt;

use crate::model::common::identifier::{IdentBase, Identifiable};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FlowBlockType {
    Sequential,
    Parallel,
    CondIf,
    CondElif,
    ZeroCond,
    WhileLoop,
    DoWhile,
    CounterLoop,
}

impl FlowBlockType {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Sequential   => "SEQUENTIAL",
            Self::Parallel     => "PARALLEL",
            Self::CondIf       => "COND_IF",
            Self::CondElif     => "COND_ELIF",
            Self::ZeroCond     => "ZERO_COND",
            Self::WhileLoop    => "WHILE_LOOP",
            Self::DoWhile      => "DO_WHILE",
            Self::CounterLoop  => "COUNTER_LOOP",
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
    ident_base: IdentBase,
    block_type: FlowBlockType,
}

impl FlowBlockIdent {
    pub fn new(block_type: FlowBlockType, name: &str) -> Self {
        Self {
            ident_base: IdentBase::new(false, name),
            block_type,
        }
    }

    pub fn get_ident_base     (&self)     -> &IdentBase     { &self.ident_base }
    pub fn get_ident_base_mut (&mut self) -> &mut IdentBase { &mut self.ident_base }
    pub fn get_block_type     (&self)     -> FlowBlockType  { self.block_type }

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
    fn default() -> Self { Self::new(FlowBlockType::Sequential, "") }
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
