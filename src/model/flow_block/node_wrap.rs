use crate::model::hw_component::common::hcp_ident::HcpIdent;
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

    // WARNING: entrance nodes placed into NodeWrap must NOT be assigned by the schematic that
    // builds the result. They are handed to the parent level, which assigns them. Only internal
    // nodes (exit, intermediates, sub-wrap entrances already consumed into the block) may be
    // assigned before returning.
    pub fn with_entrances(entrances_i: &[NcpIdent], exit_i: NcpIdent, cycle_used: i32) -> Self {
        let mut w = Self::new();
        w.add_entrance_nodes_i(entrances_i);
        w.set_exit_node_i(exit_i);
        w.set_cycle_used(cycle_used);
        w
    }

    pub fn with_single_entrance(entrance_i: NcpIdent, exit_i: NcpIdent, cycle_used: i32) -> Self {
        let mut w = Self::new();
        w.add_entrance_node_i(entrance_i);
        w.set_exit_node_i(exit_i);
        w.set_cycle_used(cycle_used);
        w
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

    pub fn add_dep_to_entrances(&self, arena: &mut ModelArena, dep_i: NcpIdent, cond_i: Option<HcpIdent>) {
        for &entrance_i in &self.entrance_nodes_i {
            arena.add_depend_node_to_ncp(entrance_i, dep_i, cond_i);
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
