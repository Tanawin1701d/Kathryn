use crate::model::common::identifier::IdentBase;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum NodeType {
    Asm,
    State,
    Syn,
    Pseudo,
    Dummy,
    Opr,
    Start,
    WaitCond,
    WaitCycle,
    Counter,
}

impl NodeType {
    pub fn as_str(&self) -> &'static str {
        match self {
            NodeType::Asm       => "ASM_NODE",
            NodeType::State     => "STATE_NODE",
            NodeType::Syn       => "SYN_NODE",
            NodeType::Pseudo    => "PSEUDO_NODE",
            NodeType::Dummy     => "DUMMY_NODE",
            NodeType::Opr       => "OPR_NODE",
            NodeType::Start     => "START_NODE",
            NodeType::WaitCond  => "WAITCOND_NODE",
            NodeType::WaitCycle => "WAITCYCLE_NODE",
            NodeType::Counter   => "COUNTER_NODE",
        }
    }
}

impl std::fmt::Display for NodeType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(self.as_str())
    }
}


#[derive(Clone, Debug, PartialEq, Eq)]
pub struct NcpIdent {
    ident_base: IdentBase,
    node_type: NodeType,
}