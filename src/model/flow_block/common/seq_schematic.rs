use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::FlowBlockIdent;
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

#[derive(Clone, Debug)]
enum SequenceEle {
    Basic    { block_i     : Option<FlowBlockIdent>, // Some => asm_node_i resolved from summarize_as_node at gen_node time (BasicNodeFlow sub-block)
               asm_node_i  : Option<NcpIdent>,       // the assignment node (None until resolved when block_i is Some)
               state_node_i: Option<NcpIdent>,       // the state of this phrase
    },

    SubBlock { block_i   : FlowBlockIdent,
               node_wrap : Option<NodeWrap> },
}

impl SequenceEle {
    /// Materialise this element's runtime nodes.  Basic: wrap the AsmNode (or
    /// the resolved summary of a BasicNodeFlow sub-block) in a fresh StateNode
    /// that inherits the parent block's ext_trigger_node.  SubBlock: build the
    /// child block's NodeWrap summary so its entrance/exit nodes are known.
    fn gen_node(&mut self, block_id: u64, idx: usize, base: &mut FlowBlockBase, arena: &mut ModelArena) {
        match self {
            Self::Basic { block_i, asm_node_i, state_node_i } => {
                // BasicNodeFlow sub-block: resolve the summarised AsmNode now that
                // the child block has been built.
                if let Some(virtual_block_i) = block_i {
                    let virtual_block = arena.take_flow_block(*virtual_block_i);
                    let summarized_asm_node = virtual_block.summarize_as_node();
                    arena.replace_back_flow_block(virtual_block);
                    assert_eq!(summarized_asm_node.get_node_type(), NodeType::Asm,
                               "SequenceEle::Basic: summarize_as_node must yield an AsmNode for BasicNodeFlow sub-block");
                    *asm_node_i = Some(summarized_asm_node);
                }
                let asm_i = asm_node_i.expect("SequenceEle::Basic: asm_node_i unresolved \
                                                        (add_node forgot the AsmNode, or add_basic_block's block has no summary)");
                let state_i = arena.make_state_node(
                    &format!("seq_state_{}_{}", block_id, idx),
                );
                arena.init_node_trigger(state_i, base.get_ext_trigger_node(), false);
                arena.add_depend_node_to_ncp(asm_i, state_i, None);
                base.add_sys_node(state_i);
                *state_node_i = Some(state_i);
            }
            Self::SubBlock { block_i, node_wrap } => {
                *node_wrap = Some(arena.summarize_flow_block(*block_i));
            }
        }
    }

    /// Contribute this element's cycle cost to the parent block's cycle detector
    /// so the surrounding NodeWrap can compute its own vertical cycle count.
    fn add_to_cycle_det(&self, det: &mut NodeWrapCycleDet, arena: &ModelArena) {
        match self {
            Self::Basic    { state_node_i, .. } =>
                det.add_cycle(arena.get_node_cycle_used(&state_node_i.expect("state_node_i not generated"))),
            Self::SubBlock { node_wrap, .. }   =>
                det.add_cycle(node_wrap.as_ref().expect("not generated").get_cycle_used()),
        }
    }


    /// Entry-side nodes that should receive an incoming dep edge — Basic has
    /// a single StateNode entrance; SubBlock fans out across the child block's
    /// summarised entrance set.
    fn entrance_nodes(&self) -> Vec<NcpIdent> {
        match self {
            Self::Basic    { state_node_i, .. } => vec![state_node_i.expect("not generated")],
            Self::SubBlock { node_wrap, .. }    => node_wrap.as_ref().expect("not generated").get_entrance_nodes_i().to_vec(),
        }
    }

    /// Wire `src_i` (optionally cond-gated) as a dep onto this element's
    /// entrance node(s).  For SubBlock the dep is fanned out across every
    /// entrance so the whole child block waits on `src_i`.
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

    /// Single exit node of this element — the StateNode for Basic, the
    /// sub-block summary's exit node for SubBlock — used as the dep source
    /// for the next element in the sequence.
    fn exit_node(&self) -> NcpIdent {
        match self {
            Self::Basic    { state_node_i, .. } => state_node_i.expect("not generated"),
            Self::SubBlock { node_wrap, .. }    => node_wrap.as_ref().expect("not generated").get_exit_node_i(),
        }
    }

    /// Run the per-element assign step.  Basic: assign the wrapping StateNode
    /// (skip on the first block — it inherits triggers from the parent) and
    /// then gate the AsmNode through that state.  SubBlock: forward the
    /// entrance-node assign into the child block, also skipped when first.
    fn assign_block(&self, arena: &mut ModelArena, is_first_block: bool) {
        match self {
            Self::Basic { asm_node_i, state_node_i, .. } => {
                arena.assign_ncp_node(state_node_i.expect("not generated"), true, !is_first_block);

                arena.assign_asm_from_state_node(asm_node_i.expect("asm_node_i unresolved"));
            }
            Self::SubBlock { node_wrap, .. } => {
                if is_first_block { return; }
                node_wrap.as_ref().expect("not generated").assign_entrance_nodes(arena);
            }
        }
    }
}

// ---- SeqSchematic -----------------------------------------------------------
// Linear sequence wiring helper.  Owns the ordered list of SequenceEle entries
// (Basic asm-node or SubBlock) and turns them into a chain of State / Sync nodes
// during `build`.  Lives by value inside FlowBlockSeq; not arena-stored.

#[derive(Clone, Debug, Default)]
pub struct SeqSchematic {
    elements: Vec<SequenceEle>,
}

impl SeqSchematic {
    pub fn new() -> Self { Self { elements: Vec::new() } }

    pub fn add_asm_node(&mut self, node_i: NcpIdent) {
        self.elements.push(SequenceEle::Basic {
            block_i      : None,
            asm_node_i   : Some(node_i),
            state_node_i : None,
        });
    }

    /// Push a BasicNodeFlow-style sub-block whose summary AsmNode will be
    /// resolved at build time via `summarize_as_node`.
    pub fn add_basic_block(&mut self, block_i: FlowBlockIdent) {
        self.elements.push(SequenceEle::Basic {
            block_i      : Some(block_i),
            asm_node_i   : None,
            state_node_i : None,
        });
    }

    pub fn add_sub_block(&mut self, block_i: FlowBlockIdent) {
        self.elements.push(SequenceEle::SubBlock { block_i, node_wrap: None });
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        // ---- stage 0: sanity checks --------------------------------------
        assert!(!self.elements.is_empty(), "seq flow block has no element");
        assert!(base.get_con_blocks_i().is_empty(), "seq flow block does not support con blocks");

        let block_id      = base.get_ident().get_global_id();
        let mut cycle_det = NodeWrapCycleDet::new();

        // ---- stage 1: materialise per-element nodes ----------------------
        //   - Basic   : create the wrapping StateNode (and resolve summary AsmNode for BasicNodeFlow sub-blocks)
        //   - SubBlock: summarise the child block so its entrance/exit are known
        // Each element also folds its cycle cost into cycle_det.
        for (idx, ele) in self.elements.iter_mut().enumerate() {
            ele.gen_node(block_id, idx, base, arena);
            ele.add_to_cycle_det(&mut cycle_det, arena);
        }

        // ---- stage 2: link the linear chain ------------------------------
        // Each element[i+1]'s entrance(s) depend on element[i]'s exit node.
        for idx in 0..self.elements.len().saturating_sub(1) {
            let src_i = self.elements[idx].exit_node();
            self.elements[idx + 1].add_depend_node(src_i, None, arena);
        }

        // ---- stage 3: hook the int-start signal --------------------------
        // If the enclosing block exposes an int-start node, gate the very first
        // element on it; this introduces an unpredictable cycle delta.
        if let Some(int_start_i) = base.get_int_node(ExtSigType::Start) {
            self.elements[0].add_depend_node(int_start_i, None, arena);
            cycle_det.add_cycle(IN_CONSIST_CYCLE_USED);
        }

        // ---- stage 4: per-element assign ---------------------------------
        // Basic: assign the StateNode (skipped on first — inherits parent triggers)
        //        then the AsmNode through it.
        // SubBlock: forward entrance-node assigns into the child (also skipped first).
        for (idx, ele) in self.elements.iter().enumerate() {
            ele.assign_block(arena, idx == 0);
        }

        // ---- stage 5: summarise as a NodeWrap ----------------------------
        // Entrance set = element[0]'s entrances, exit = last element's exit.
        NodeWrap::with_entrances(
            &self.elements[0].entrance_nodes(),
            self.elements.last().expect("checked above").exit_node(),
            cycle_det.get_cycle_vertical(),
        )
    }
}
