use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::asm_meta_helper::AssignMetaIfPool;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::add_logic_with_output;
use crate::model::nodes::ncp_ident::NcpIdent;

/// Master ZIF branch — owns the chained zelif / zelse `con_blocks`.
/// Runs both the per-branch GRP pass on its own basic nodes and the
/// cross-branch merge across all continuation blocks.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroCondIf {
    base      : FlowBlockBase,
    condition : HcpIdent,
    grp_asms  : Vec<AssignMeta>,
}

impl Default for FlowBlockZeroCondIf {
    fn default() -> Self { Self::new("", HcpIdent::default()) }
}

impl FlowBlockZeroCondIf {
    pub fn new(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::ZeroCondIf, FlowBlockJoinPolicy::ExtFlow, name),
            condition: cond_i,
            grp_asms : Vec::new(),
        }
    }
}

impl FlowBlock for FlowBlockZeroCondIf {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("zero-cond-if block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_con_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_cond_if(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        // Per-branch GRP pass on the master's own basic nodes.
        self.grp_asms = self.base.gen_unified_asm_meta_from_all_basic_nodes(arena);

        // Cross-branch merge via AssignMetaIfPool.
        let id        = self.base.get_ident().get_global_id();
        let self_cond = self.condition;
        let mut pool  = AssignMetaIfPool::default();
        pool.insert_asms(arena, Some(self_cond), self_cond, &self.grp_asms);

        // prev_false = ~self_cond  (same pattern as cond_schematic.rs)
        let inv_self = arena.make_expression(
            &format!("zcif_not_cond_{}", id),
            LogicOp::BitwiseInvr, self_cond, HcpIdent::default(), None, None,
        );
        let mut prev_false: Option<HcpIdent> = Some(inv_self);

        let con_blocks_i: Vec<FlowBlockIdent> = self.base.get_con_blocks_i().to_vec();
        for (i, con_block_i) in con_blocks_i.into_iter().enumerate() {
            let con = arena.take_flow_block_zero_cond_elif(con_block_i);
            let con_cond = con.get_condition();

            let con_cond_abs = match con_cond {
                Some(elif_cond) => {
                    // zelif: abs = elif_cond & prev_false; advance prev_false
                    let gated    = add_logic_with_output(arena, Some(elif_cond), prev_false, LogicOp::BitwiseAnd);
                    let inv_elif = arena.make_expression(
                        &format!("zcif_not_elif_{}_{}", id, i),
                        LogicOp::BitwiseInvr, elif_cond, HcpIdent::default(), None, None,
                    );
                    prev_false = add_logic_with_output(arena, prev_false, Some(inv_elif), LogicOp::BitwiseAnd);
                    gated.unwrap_or_default()
                }
                None => {
                    // zelse: abs = prev_false (consume)
                    prev_false.take().unwrap_or_default()
                }
            };

            pool.insert_asms(arena, con_cond, con_cond_abs, con.get_grp_asms());
            arena.replace_back_flow_block_zero_cond_elif(con);
        }

        self.grp_asms = pool.gen_assign_metas(arena);
    }

    fn summarize_block(&self) -> NodeWrap {
        todo!("ZIF summarize not implemented — requires controller-level extraction before synthesis")
    }

    fn get_con_condition(&self) -> Option<HcpIdent> {
        Some(self.condition)
    }

    fn get_extract_node(&self) -> NcpIdent {
        todo!("get_unified_node for ZeroCondIf — requires controller-level extraction")
    }
}

impl Identifiable for FlowBlockZeroCondIf {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
