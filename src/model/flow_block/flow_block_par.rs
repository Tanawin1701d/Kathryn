use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase, ExtSigType};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockType};
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
struct FlowBlockPar {
    base: FlowBlockBase,
    node_wraps_of_sub_blocks: Vec<NodeWrap>,
    result_node_wrap: Option<NodeWrap>,
    basic_state_node: Option<NcpIdent>,
    syn_node: Option<NcpIdent>,
    pseudo_exit_node: Option<NcpIdent>,
    cycle_used: i32,
}

impl FlowBlockPar {
    fn new(block_type: FlowBlockType, is_user_com: bool, name: &str) -> Self {
        Self {
            base: FlowBlockBase::new(block_type, is_user_com, name),
            node_wraps_of_sub_blocks: Vec::new(),
            result_node_wrap: None,
            basic_state_node: None,
            syn_node: None,
            pseudo_exit_node: None,
            cycle_used: IN_CONSIST_CYCLE_USED,
        }
    }

    fn add_element_in_flow_block(&mut self, node: NcpIdent) {
        self.base.add_basic_node(node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_sub_flow_block(block);
    }

    fn build_common_par_hw(&mut self, arena: &mut ModelArena) {
        assert!(
            !self.base.get_basic_nodes_i().is_empty() || !self.base.get_sub_blocks_i().is_empty(),
            "parallel flow block has no element"
        );
        assert!(
            self.base.get_con_blocks_i().is_empty(),
            "parallel flow block does not support con blocks"
        );

        if !self.base.get_basic_nodes_i().is_empty() {
            let state = arena.make_state_node(
                &format!("par_state_{}", self.base.get_ident().get_global_id()),
                self.base.get_clock_mode(),
            );
            if let Some(rst) = self.base.get_int_node(ExtSigType::Reset) {
                arena.set_ncp_int_reset_node(state, rst);
            }
            if let Some(hold) = self.base.get_hold_node() {
                arena.set_ncp_hold_node(state, hold);
            }
            for asm in self.base.get_basic_nodes_i() {
                arena.add_slave_asm_to_state_node(state, *asm, None);
            }
            self.base.add_sys_node(state);
            self.basic_state_node = Some(state);
        }

        self.node_wraps_of_sub_blocks = self
            .base
            .get_sub_blocks_i()
            .iter()
            .map(|block| arena.summarize_flow_block(*block))
            .collect();
        self.base
            .gen_sum_force_exit_node(&self.node_wraps_of_sub_blocks, arena);

        let mut cycle_det = NodeWrapCycleDet::new();
        if let Some(state) = self.basic_state_node {
            cycle_det.add_cycle(arena.get_node_cycle_used(&state));
        }
        for wrap in &self.node_wraps_of_sub_blocks {
            cycle_det.add_cycle(wrap.get_cycle_used());
        }
        self.cycle_used = cycle_det.get_max_cycle_horizon();

        if let Some(int_start) = self.base.get_int_node(ExtSigType::Start) {
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

    fn assign_entrance_nodes(&self, arena: &mut ModelArena, assign_sub_entrances: bool) {
        if let Some(state) = self.basic_state_node {
            arena.assign_ncp_node(state);
            for asm in self.base.get_basic_nodes_i() {
                arena.assign_asm_from_state_node(*asm);
            }
        }
        if !assign_sub_entrances {
            return;
        }
        for wrap in &self.node_wraps_of_sub_blocks {
            for entrance in wrap.get_entrance_nodes_i() {
                arena.assign_ncp_node(*entrance);
            }
        }
    }

    fn result_with_exit(&self, exit_node: NcpIdent) -> NodeWrap {
        let mut result = NodeWrap::new();
        if let Some(state) = self.basic_state_node {
            result.add_entrance_node_i(state);
        }
        for wrap in &self.node_wraps_of_sub_blocks {
            result.get_entrance_nodes_i_from(wrap);
        }
        result.set_exit_node_i(exit_node);
        result.set_cycle_used(self.cycle_used);
        if let Some(force) = self.base.get_force_exit_node() {
            result.set_force_exit_node(force);
        }
        result
    }

    fn amount_of_paths(&self) -> usize {
        usize::from(self.basic_state_node.is_some()) + self.node_wraps_of_sub_blocks.len()
    }

    fn path_exit_nodes(&self) -> Vec<(NcpIdent, i32)> {
        let mut nodes = Vec::new();
        if let Some(state) = self.basic_state_node {
            nodes.push((
                state,
                self.basic_state_node
                    .map(|_| 1)
                    .unwrap_or(IN_CONSIST_CYCLE_USED),
            ));
        }
        nodes.extend(
            self.node_wraps_of_sub_blocks
                .iter()
                .map(|wrap| (wrap.get_exit_node_i(), wrap.get_cycle_used())),
        );
        nodes
    }

    fn exit_matching_cycle(&self) -> Option<NcpIdent> {
        if self.cycle_used < 0 {
            return None;
        }
        self.path_exit_nodes()
            .into_iter()
            .find_map(|(node, cycle)| {
                if cycle == self.cycle_used {
                    Some(node)
                } else {
                    None
                }
            })
    }

    fn any_exit(&self) -> Option<NcpIdent> {
        self.path_exit_nodes()
            .into_iter()
            .map(|(node, _)| node)
            .next()
    }

    fn summarize_block(&self) -> NodeWrap {
        self.result_node_wrap
            .clone()
            .expect("flow block has not been built")
    }
}

#[derive(Clone, Debug)]
pub struct FlowBlockParAuto {
    inner: FlowBlockPar,
}

impl Default for FlowBlockParAuto {
    fn default() -> Self {
        Self::new(false, "")
    }
}

impl FlowBlockParAuto {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        Self {
            inner: FlowBlockPar::new(FlowBlockType::ParallelAutoSync, is_user_com, name),
        }
    }
}

impl FlowBlock for FlowBlockParAuto {
    fn get_base(&self) -> &FlowBlockBase { &self.inner.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.inner.base }

    fn add_element_in_flow_block(&mut self, node: NcpIdent) {
        self.inner.add_element_in_flow_block(node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.inner.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_par_auto(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.inner.build_common_par_hw(arena);
        let assign_sub_entrances = self.inner.base.has_int_start();
        self.inner.assign_entrance_nodes(arena, assign_sub_entrances);

        if self.inner.cycle_used == IN_CONSIST_CYCLE_USED && self.inner.amount_of_paths() > 1 {
            let syn = arena.make_syn_node(
                &format!("par_syn_{}", self.inner.base.get_ident().get_global_id()),
                self.inner.amount_of_paths() as i32,
                self.inner.base.get_clock_mode(),
            );
            if let Some(rst) = self.inner.base.get_int_node(ExtSigType::Reset) {
                arena.set_ncp_int_reset_node(syn, rst);
            }
            for (exit, _) in self.inner.path_exit_nodes() {
                arena.add_depend_node_to_ncp(syn, exit, None);
            }
            self.inner.base.add_sys_node(syn);
            arena.assign_ncp_node(syn);
            self.inner.syn_node = Some(syn);
        }

        let exit = self
            .inner
            .syn_node
            .or_else(|| self.inner.exit_matching_cycle())
            .or_else(|| {
                assert_eq!(
                    self.inner.amount_of_paths(),
                    1,
                    "parallel auto-sync cannot choose exit node"
                );
                self.inner.any_exit()
            })
            .expect("parallel block exit node was not found");
        self.inner.result_node_wrap = Some(self.inner.result_with_exit(exit));
    }

    fn summarize_block(&self) -> NodeWrap {
        self.inner.summarize_block()
    }
}

impl Identifiable for FlowBlockParAuto {
    fn get_ident_base(&self) -> &IdentBase {
        self.inner.base.get_ident_ref().get_ident_base()
    }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase {
        self.inner.base.get_ident_mut().get_ident_base_mut()
    }
    fn build_unique_name(&mut self) -> &str {
        self.inner.base.get_ident_mut().build_unique_name()
    }
}

#[derive(Clone, Debug)]
pub struct FlowBlockParNoSync {
    inner: FlowBlockPar,
}

impl Default for FlowBlockParNoSync {
    fn default() -> Self {
        Self::new(false, "")
    }
}

impl FlowBlockParNoSync {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        Self {
            inner: FlowBlockPar::new(FlowBlockType::ParallelNoSync, is_user_com, name),
        }
    }
}

impl FlowBlock for FlowBlockParNoSync {
    fn get_base(&self) -> &FlowBlockBase { &self.inner.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.inner.base }

    fn add_element_in_flow_block(&mut self, node: NcpIdent) {
        self.inner.add_element_in_flow_block(node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.inner.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_par_no_sync(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.inner.build_common_par_hw(arena);
        let assign_sub_entrances = self.inner.base.has_int_start();
        self.inner.assign_entrance_nodes(arena, assign_sub_entrances);

        let exit = if let Some(exit) = self.inner.exit_matching_cycle() {
            exit
        } else if self.inner.amount_of_paths() == 1 {
            self.inner
                .any_exit()
                .expect("parallel block exit node was not found")
        } else {
            let pseudo = arena.make_pseudo_node("par_no_sync_exit", 1, LogicOp::BitwiseOr);
            for (exit, _) in self.inner.path_exit_nodes() {
                arena.add_depend_node_to_ncp(pseudo, exit, None);
            }
            self.inner.base.add_sys_node(pseudo);
            arena.assign_ncp_node(pseudo);
            self.inner.pseudo_exit_node = Some(pseudo);
            pseudo
        };

        self.inner.result_node_wrap = Some(self.inner.result_with_exit(exit));
    }

    fn summarize_block(&self) -> NodeWrap {
        self.inner.summarize_block()
    }
}

impl Identifiable for FlowBlockParNoSync {
    fn get_ident_base(&self) -> &IdentBase {
        self.inner.base.get_ident_ref().get_ident_base()
    }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase {
        self.inner.base.get_ident_mut().get_ident_base_mut()
    }
    fn build_unique_name(&mut self) -> &str {
        self.inner.base.get_ident_mut().build_unique_name()
    }
}
