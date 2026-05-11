use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct NodeWrap {
    entrance_nodes_i: Vec<NcpIdent>,
    exit_node       : Option<NcpIdent>,
    cycle_used      : i32,
}

impl NodeWrap {
    pub fn new() -> Self {
        Self {
            entrance_nodes_i: Vec::new(),
            exit_node       : None,
            cycle_used      : IN_CONSIST_CYCLE_USED,
        }
    }

    pub fn add_entrance_node_i      (&mut self, node : NcpIdent   ) { self.entrance_nodes_i.push(node); }
    pub fn add_entrance_nodes_i     (&mut self, nodes: &[NcpIdent]) { self.entrance_nodes_i.extend_from_slice(nodes);}
    pub fn get_entrance_nodes_i_from(&mut self, other: &NodeWrap  ) { self.add_entrance_nodes_i(other.get_entrance_nodes_i());}

    pub fn set_exit_node_i(&mut self, node: NcpIdent) { self.exit_node = Some(node);}
    pub fn set_cycle_used (&mut self, cycle: i32) {
        assert!(cycle == IN_CONSIST_CYCLE_USED || cycle >= 0);
        self.cycle_used = cycle;
    }

    pub fn get_entrance_nodes_i(&self) -> &[NcpIdent] { &self.entrance_nodes_i }
    pub fn get_exit_node_i     (&self) -> NcpIdent    { self.exit_node.expect("NodeWrap exit node is not set") }
    pub fn get_cycle_used      (&self) -> i32         { self.cycle_used }

    pub fn assign_entrance_nodes(&self, arena: &mut ModelArena) {
        for entrance_i in &self.entrance_nodes_i {
            arena.assign_ncp_node(*entrance_i);
        }
    }
}

#[derive(Default)]
pub struct NodeWrapCycleDet {
    samples: Vec<i32>,
}

impl NodeWrapCycleDet {
    pub fn new() -> Self {
        Self {
            samples: Vec::new(),
        }
    }
    pub fn add_cycle(&mut self, cycle: i32) { self.samples.push(cycle); }

    pub fn get_max_cycle_horizon(&self) -> i32 {
        assert!(!self.samples.is_empty());
        let mut max_cycle = 0;
        for cycle in &self.samples {
            if *cycle == IN_CONSIST_CYCLE_USED {
                return IN_CONSIST_CYCLE_USED;
            }
            max_cycle = max_cycle.max(*cycle);
        }
        max_cycle
    }

    pub fn get_cycle_vertical(&self) -> i32 {
        assert!(!self.samples.is_empty());
        let mut cycle_used = 0;
        for cycle in &self.samples {
            if *cycle == IN_CONSIST_CYCLE_USED {
                return IN_CONSIST_CYCLE_USED;
            }
            cycle_used += *cycle;
        }
        cycle_used
    }

    pub fn get_same_cycle_horizon(&self) -> i32 {
        assert!(!self.samples.is_empty());
        let mut common: Option<i32> = None;
        for &cycle in &self.samples {
            if cycle == IN_CONSIST_CYCLE_USED {
                return IN_CONSIST_CYCLE_USED;
            }
            match common {
                None           => common = Some(cycle),
                Some(c) if c == cycle => {}
                Some(_) => return IN_CONSIST_CYCLE_USED,
            }
        }
        common.unwrap_or(0)
    }
}
