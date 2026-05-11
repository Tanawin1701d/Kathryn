use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::FlowBlockIdent;
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
enum SequenceEle {
    Basic    { asm_node_i  : NcpIdent      ,  // the assignment node to variable a |= b
               state_node_i: Option<NcpIdent> // the state of this phrase
    },

    SubBlock { block_i   : FlowBlockIdent,
               node_wrap : Option<NodeWrap> },
}

impl SequenceEle {
    fn gen_node(&mut self, block_id: u64, idx: usize, base: &mut FlowBlockBase, arena: &mut ModelArena) {
        match self {
            Self::Basic { asm_node_i, state_node_i } => {
                let state_i = arena.make_state_node(
                    &format!("seq_state_{}_{}", block_id, idx),
                );
                if let Some(rst_i)  = base.get_int_node(ExtSigType::Reset) { arena.set_ncp_int_reset_node(state_i, rst_i); }
                if let Some(hold_i) = base.get_hold_node()                  { arena.set_ncp_hold_node(state_i, hold_i);     }
                arena.add_slave_asm_to_state_node(state_i, *asm_node_i, None);
                base.add_sys_node(state_i);
                *state_node_i = Some(state_i);
            }
            Self::SubBlock { block_i, node_wrap } => {
                *node_wrap = Some(arena.summarize_flow_block(*block_i));
            }
        }
    }

    fn add_to_cycle_det(&self, det: &mut NodeWrapCycleDet, arena: &ModelArena) {
        match self {
            Self::Basic    { asm_node_i, .. }  => det.add_cycle(arena.get_node_cycle_used(asm_node_i)),
            Self::SubBlock { node_wrap, .. }   => det.add_cycle(node_wrap.as_ref().expect("not generated").get_cycle_used()),
        }
    }

    fn finish_node(&self) -> NcpIdent {
        match self {
            Self::Basic    { state_node_i, .. } => state_node_i.expect("not generated"),
            Self::SubBlock { node_wrap, .. }    => node_wrap.as_ref().expect("not generated").get_exit_node_i(),
        }
    }

    fn entrance_nodes(&self) -> Vec<NcpIdent> {
        match self {
            Self::Basic    { state_node_i, .. } => vec![state_node_i.expect("not generated")],
            Self::SubBlock { node_wrap, .. }    => node_wrap.as_ref().expect("not generated").get_entrance_nodes_i().to_vec(),
        }
    }

    fn add_depend_node(&self, src_i: NcpIdent, cond: Option<HcpIdent>, arena: &mut ModelArena) {
        match self {
            Self::Basic { state_node_i, .. } => {
                arena.add_depend_node_to_ncp(state_node_i.expect("not generated"), src_i, cond);
            }
            Self::SubBlock { node_wrap, .. } => {
                for entrance_i in node_wrap.as_ref().expect("not generated").get_entrance_nodes_i() {
                    arena.add_depend_node_to_ncp(*entrance_i, src_i, cond);
                }
            }
        }
    }

    fn assign_block(&self, arena: &mut ModelArena, is_first_block: bool) {
        match self {
            Self::Basic { asm_node_i, state_node_i } => {
                if !is_first_block {
                    arena.assign_ncp_node(state_node_i.expect("not generated"));
                }
                arena.assign_asm_from_state_node(*asm_node_i);
            }
            Self::SubBlock { node_wrap, .. } => {
                if is_first_block { return; }
                node_wrap.as_ref().expect("not generated").assign_entrance_nodes(arena);
            }
        }
    }
}

#[derive(Clone, Debug, Default)]
pub struct SeqSchematic {
    elements: Vec<SequenceEle>,
}

impl SeqSchematic {
    pub fn new() -> Self { Self { elements: Vec::new() } }

    pub fn add_node(&mut self, node_i: NcpIdent) {
        self.elements.push(SequenceEle::Basic { asm_node_i: node_i, state_node_i: None });
    }

    pub fn add_sub_block(&mut self, block_i: FlowBlockIdent) {
        self.elements.push(SequenceEle::SubBlock { block_i, node_wrap: None });
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        assert!(!self.elements.is_empty(), "seq flow block has no element");
        assert!(base.get_con_blocks_i().is_empty(), "seq flow block does not support con blocks");

        let block_id = base.get_ident().get_global_id();
        let mut cycle_det = NodeWrapCycleDet::new();

        for (idx, ele) in self.elements.iter_mut().enumerate() {
            ele.gen_node(block_id, idx, base, arena);
            ele.add_to_cycle_det(&mut cycle_det, arena);
        }

        for idx in 0..self.elements.len().saturating_sub(1) {
            let src_i = self.elements[idx].finish_node();
            self.elements[idx + 1].add_depend_node(src_i, None, arena);
        }

        if let Some(int_start_i) = base.get_int_node(ExtSigType::Start) {
            self.elements[0].add_depend_node(int_start_i, None, arena);
            cycle_det.add_cycle(IN_CONSIST_CYCLE_USED);
        }

        for (idx, ele) in self.elements.iter().enumerate() {
            ele.assign_block(arena, idx == 0);
        }

        let mut result = NodeWrap::new();
        result.add_entrance_nodes_i(&self.elements[0].entrance_nodes());
        result.set_exit_node_i(self.elements.last().expect("checked above").finish_node());
        result.set_cycle_used(cycle_det.get_cycle_vertical());
        result
    }
}
