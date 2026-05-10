use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockType};
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
enum SequenceEle {
    Basic {
        asm_node: NcpIdent,
        state_node: Option<NcpIdent>,
    },
    FlowBlock {
        block: FlowBlockIdent,
        node_wrap: Option<NodeWrap>,
    },
}

impl SequenceEle {
    fn gen_node(
        &mut self,
        block_id: u64,
        idx: usize,
        base: &mut FlowBlockBase,
        arena: &mut ModelArena,
    ) {
        match self {
            Self::Basic {
                asm_node,
                state_node,
            } => {
                let state = arena.make_state_node(
                    &format!("seq_state_{}_{}", block_id, idx),
                    base.get_clock_mode(),
                );
                if let Some(rst) = base.get_int_node(crate::model::flow_block::ExtSigType::Reset) {
                    arena.set_ncp_int_reset_node(state, rst);
                }
                if let Some(hold) = base.get_hold_node() {
                    arena.set_ncp_hold_node(state, hold);
                }
                arena.add_slave_asm_to_state_node(state, *asm_node, None);
                base.add_sys_node(state);
                *state_node = Some(state);
            }
            Self::FlowBlock { block, node_wrap } => {
                *node_wrap = Some(arena.summarize_flow_block(*block));
            }
        }
    }

    fn add_to_cycle_det(&self, det: &mut NodeWrapCycleDet, arena: &ModelArena) {
        match self {
            Self::Basic { asm_node, .. } => det.add_cycle(arena.get_node_cycle_used(asm_node)),
            Self::FlowBlock { node_wrap, .. } => {
                det.add_cycle(
                    node_wrap
                        .as_ref()
                        .expect("flow element was not generated")
                        .get_cycle_used(),
                );
            }
        }
    }

    fn state_finish_node(&self) -> NcpIdent {
        match self {
            Self::Basic { state_node, .. } => state_node.expect("state node was not generated"),
            Self::FlowBlock { node_wrap, .. } => node_wrap
                .as_ref()
                .expect("flow element was not generated")
                .get_exit_node_i(),
        }
    }

    fn entrance_nodes(&self) -> Vec<NcpIdent> {
        match self {
            Self::Basic { state_node, .. } => {
                vec![state_node.expect("state node was not generated")]
            }
            Self::FlowBlock { node_wrap, .. } => node_wrap
                .as_ref()
                .expect("flow element was not generated")
                .get_entrance_nodes_i()
                .to_vec(),
        }
    }

    fn force_exit_node(&self) -> Option<NcpIdent> {
        match self {
            Self::Basic { .. } => None,
            Self::FlowBlock { node_wrap, .. } => node_wrap
                .as_ref()
                .expect("flow element was not generated")
                .get_force_exit_node(),
        }
    }

    fn add_depend_node(&self, src: NcpIdent, cond: Option<HcpIdent>, arena: &mut ModelArena) {
        match self {
            Self::Basic { state_node, .. } => {
                arena.add_depend_node_to_ncp(
                    state_node.expect("state node was not generated"),
                    src,
                    cond,
                );
            }
            Self::FlowBlock { node_wrap, .. } => {
                for entrance in node_wrap
                    .as_ref()
                    .expect("flow element was not generated")
                    .get_entrance_nodes_i()
                {
                    arena.add_depend_node_to_ncp(*entrance, src, cond);
                }
            }
        }
    }

    fn assign_entrance_nodes(&self, arena: &mut ModelArena, force_reassign: bool) {
        match self {
            Self::Basic {
                asm_node,
                state_node,
            } => {
                let state = state_node.expect("state node was not generated");
                arena.assign_ncp_node(state);
                arena.assign_asm_from_state_node(*asm_node);
            }
            Self::FlowBlock { node_wrap, .. } => {
                if !force_reassign {
                    return;
                }
                for entrance in node_wrap
                    .as_ref()
                    .expect("flow element was not generated")
                    .get_entrance_nodes_i()
                {
                    arena.assign_ncp_node(*entrance);
                }
            }
        }
    }
}

#[derive(Clone, Debug)]
pub struct FlowBlockSeq {
    base: FlowBlockBase,
    seq_elements: Vec<SequenceEle>,
    result_node_wrap: Option<NodeWrap>,
}

impl Default for FlowBlockSeq {
    fn default() -> Self {
        Self::new(false, "")
    }
}

impl FlowBlockSeq {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        Self {
            base: FlowBlockBase::new(FlowBlockType::Sequential, is_user_com, name),
            seq_elements: Vec::new(),
            result_node_wrap: None,
        }
    }
}

impl FlowBlock for FlowBlockSeq {
    fn get_base(&self) -> &FlowBlockBase { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, node: NcpIdent) {
        self.seq_elements.push(SequenceEle::Basic { asm_node: node, state_node: None });
        self.base.add_basic_node(node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.seq_elements.push(SequenceEle::FlowBlock { block, node_wrap: None });
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_seq(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        assert!(
            !self.seq_elements.is_empty(),
            "seq flow block has no element"
        );
        assert!(
            self.base.get_con_blocks_i().is_empty(),
            "seq flow block does not support con blocks"
        );

        let block_id = self.base.get_ident().get_global_id();
        let mut cycle_det = NodeWrapCycleDet::new();

        for (idx, ele) in self.seq_elements.iter_mut().enumerate() {
            ele.gen_node(block_id, idx, &mut self.base, arena);
            ele.add_to_cycle_det(&mut cycle_det, arena);
        }

        let force_wraps: Vec<NodeWrap> = self
            .seq_elements
            .iter()
            .filter_map(|ele| {
                ele.force_exit_node().map(|force| {
                    let mut wrap = NodeWrap::new();
                    wrap.set_force_exit_node(force);
                    wrap
                })
            })
            .collect();
        self.base.gen_sum_force_exit_node(&force_wraps, arena);

        let mut requires_assign: Vec<bool> = self
            .seq_elements
            .iter()
            .map(|ele| matches!(ele, SequenceEle::Basic { .. }))
            .collect();

        for idx in 0..self.seq_elements.len().saturating_sub(1) {
            let src = self.seq_elements[idx].state_finish_node();
            self.seq_elements[idx + 1].add_depend_node(src, None, arena);
            requires_assign[idx + 1] = true;
        }

        if let Some(int_start) = self
            .base
            .get_int_node(crate::model::flow_block::ExtSigType::Start)
        {
            self.seq_elements[0].add_depend_node(int_start, None, arena);
            requires_assign[0] = true;
        }

        for (ele, requires_assign) in self.seq_elements.iter().zip(requires_assign) {
            ele.assign_entrance_nodes(arena, requires_assign);
        }

        let mut result = NodeWrap::new();
        result.add_entrance_nodes_i(&self.seq_elements[0].entrance_nodes());
        result.set_exit_node_i(
            self.seq_elements
                .last()
                .expect("checked above")
                .state_finish_node(),
        );
        result.set_cycle_used(cycle_det.get_cycle_vertical());
        if let Some(force) = self.base.get_force_exit_node() {
            result.set_force_exit_node(force);
        }
        self.result_node_wrap = Some(result);
    }

    fn summarize_block(&self) -> NodeWrap {
        self.result_node_wrap
            .clone()
            .expect("flow block has not been built")
    }
}

impl Identifiable for FlowBlockSeq {
    fn get_ident_base(&self) -> &IdentBase {
        self.base.get_ident_ref().get_ident_base()
    }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase {
        self.base.get_ident_mut().get_ident_base_mut()
    }
    fn build_unique_name(&mut self) -> &str {
        self.base.get_ident_mut().build_unique_name()
    }
}
