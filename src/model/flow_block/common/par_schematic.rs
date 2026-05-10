use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ParSyncMode {
    AutoSync,
    NoSync,
}

#[derive(Clone, Debug)]
pub struct ParSchematic {
    sync_mode:                ParSyncMode,
    basic_state_node:         Option<NcpIdent>,
    node_wraps_of_sub_blocks: Vec<NodeWrap>,
    syn_node:                 Option<NcpIdent>,
    pseudo_exit_node:         Option<NcpIdent>,
    cycle_used:               i32,
}

impl ParSchematic {
    pub fn new(sync_mode: ParSyncMode) -> Self {
        Self {
            sync_mode,
            basic_state_node:         None,
            node_wraps_of_sub_blocks: Vec::new(),
            syn_node:                 None,
            pseudo_exit_node:         None,
            cycle_used:               IN_CONSIST_CYCLE_USED,
        }
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        self.build_common(base, arena);
        let exit = match self.sync_mode {
            ParSyncMode::AutoSync => self.build_auto_sync_exit(base, arena),
            ParSyncMode::NoSync   => self.build_no_sync_exit(base, arena),
        };
        self.build_result(base, exit)
    }

    fn build_common(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) {
        assert!(
            !base.get_basic_nodes_i().is_empty() || !base.get_sub_blocks_i().is_empty(),
            "parallel flow block has no element"
        );
        assert!(base.get_con_blocks_i().is_empty(), "parallel flow block does not support con blocks");

        if !base.get_basic_nodes_i().is_empty() {
            let state = arena.make_state_node(
                &format!("par_state_{}", base.get_ident().get_global_id()),
                base.get_clock_mode(),
            );
            if let Some(rst)  = base.get_int_node(ExtSigType::Reset) { arena.set_ncp_int_reset_node(state, rst); }
            if let Some(hold) = base.get_hold_node()                  { arena.set_ncp_hold_node(state, hold); }
            for asm in base.get_basic_nodes_i() {
                arena.add_slave_asm_to_state_node(state, *asm, None);
            }
            base.add_sys_node(state);
            self.basic_state_node = Some(state);
        }

        self.node_wraps_of_sub_blocks = base.get_sub_blocks_i().iter()
            .map(|block| arena.summarize_flow_block(*block))
            .collect();
        base.gen_sum_force_exit_node(&self.node_wraps_of_sub_blocks, arena);

        let mut cycle_det = NodeWrapCycleDet::new();
        if let Some(state) = self.basic_state_node {
            cycle_det.add_cycle(arena.get_node_cycle_used(&state));
        }
        for wrap in &self.node_wraps_of_sub_blocks {
            cycle_det.add_cycle(wrap.get_cycle_used());
        }
        self.cycle_used = cycle_det.get_max_cycle_horizon();

        if let Some(int_start) = base.get_int_node(ExtSigType::Start) {
            if let Some(state) = self.basic_state_node {
                arena.add_depend_node_to_ncp(state, int_start, None);
            }
            for wrap in &self.node_wraps_of_sub_blocks {
                for entrance in wrap.get_entrance_nodes_i() {
                    arena.add_depend_node_to_ncp(*entrance, int_start, None);
                }
            }
        }
    }

    fn assign_entrance_nodes(&self, base: &FlowBlockBase, arena: &mut ModelArena, assign_sub: bool) {
        if let Some(state) = self.basic_state_node {
            arena.assign_ncp_node(state);
            for asm in base.get_basic_nodes_i() {
                arena.assign_asm_from_state_node(*asm);
            }
        }
        if !assign_sub { return; }
        for wrap in &self.node_wraps_of_sub_blocks {
            for entrance in wrap.get_entrance_nodes_i() {
                arena.assign_ncp_node(*entrance);
            }
        }
    }

    fn amount_of_paths(&self) -> usize {
        usize::from(self.basic_state_node.is_some()) + self.node_wraps_of_sub_blocks.len()
    }

    fn path_exit_nodes(&self) -> Vec<(NcpIdent, i32)> {
        let mut nodes: Vec<(NcpIdent, i32)> = Vec::new();
        if let Some(state) = self.basic_state_node {
            nodes.push((state, 1));
        }
        nodes.extend(self.node_wraps_of_sub_blocks.iter().map(|w| (w.get_exit_node_i(), w.get_cycle_used())));
        nodes
    }

    fn exit_matching_cycle(&self) -> Option<NcpIdent> {
        if self.cycle_used < 0 { return None; }
        self.path_exit_nodes().into_iter()
            .find_map(|(node, cycle)| if cycle == self.cycle_used { Some(node) } else { None })
    }

    fn any_exit(&self) -> Option<NcpIdent> {
        self.path_exit_nodes().into_iter().map(|(node, _)| node).next()
    }

    fn build_auto_sync_exit(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NcpIdent {
        self.assign_entrance_nodes(base, arena, base.has_int_start());

        if self.cycle_used == IN_CONSIST_CYCLE_USED && self.amount_of_paths() > 1 {
            let syn = arena.make_syn_node(
                &format!("par_syn_{}", base.get_ident().get_global_id()),
                self.amount_of_paths() as i32,
                base.get_clock_mode(),
            );
            if let Some(rst) = base.get_int_node(ExtSigType::Reset) {
                arena.set_ncp_int_reset_node(syn, rst);
            }
            for (exit, _) in self.path_exit_nodes() {
                arena.add_depend_node_to_ncp(syn, exit, None);
            }
            base.add_sys_node(syn);
            arena.assign_ncp_node(syn);
            self.syn_node = Some(syn);
        }

        self.syn_node
            .or_else(|| self.exit_matching_cycle())
            .or_else(|| {
                assert_eq!(self.amount_of_paths(), 1, "parallel auto-sync cannot choose exit node");
                self.any_exit()
            })
            .expect("parallel block exit node was not found")
    }

    fn build_no_sync_exit(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NcpIdent {
        self.assign_entrance_nodes(base, arena, base.has_int_start());

        if let Some(exit) = self.exit_matching_cycle() {
            exit
        } else if self.amount_of_paths() == 1 {
            self.any_exit().expect("parallel block exit node was not found")
        } else {
            let pseudo = arena.make_pseudo_node("par_no_sync_exit", 1, LogicOp::BitwiseOr);
            for (exit, _) in self.path_exit_nodes() {
                arena.add_depend_node_to_ncp(pseudo, exit, None);
            }
            base.add_sys_node(pseudo);
            arena.assign_ncp_node(pseudo);
            self.pseudo_exit_node = Some(pseudo);
            pseudo
        }
    }

    fn build_result(&self, base: &FlowBlockBase, exit: NcpIdent) -> NodeWrap {
        let mut result = NodeWrap::new();
        if let Some(state) = self.basic_state_node {
            result.add_entrance_node_i(state);
        }
        for wrap in &self.node_wraps_of_sub_blocks {
            result.get_entrance_nodes_i_from(wrap);
        }
        result.set_exit_node_i(exit);
        result.set_cycle_used(self.cycle_used);
        if let Some(force) = base.get_force_exit_node() {
            result.set_force_exit_node(force);
        }
        result
    }
}
