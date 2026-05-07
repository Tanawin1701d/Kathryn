use crate::common::arena_base::ArenaHandle;
use crate::model::common::identifier::{IdentBase, Identifiable};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum NodeType {
    Asm,
    State,
    Syn,
    Pseudo,
    Opr,
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
            NodeType::Opr       => "OPR_NODE",
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

trait NcpIdentifiable: Identifiable {
    fn get_iden_base(&self) -> &IdentBase;
    fn get_iden_base_mut(&mut self) -> &mut IdentBase;

    fn set_arena_handler(&mut self, arena_handler: ArenaHandle) {
        self.set_arena_handle(arena_handler);
    }
}


#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct NcpIdent {
    ident_base: IdentBase,
    node_type: NodeType,
}

impl NcpIdent {
    pub fn new(node_type: NodeType, is_user_com: bool, name: &str) -> Self {
        Self {
            ident_base: IdentBase::new(is_user_com, name),
            node_type,
        }
    }

    pub fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    pub fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    pub fn get_node_type     (&self)     -> NodeType       { self.node_type }

    pub fn build_unique_ncp_name(&self) -> String {
        format!("{}_{}_{}", self.node_type,
                            self.ident_base.get_name(),
                            self.ident_base.get_global_id())
    }
}

impl Identifiable for NcpIdent {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    fn build_unique_name (&mut self) -> &str {
        let name = self.build_unique_ncp_name();
        self.ident_base.set_name(&name);
        self.ident_base.get_name()
    }
}