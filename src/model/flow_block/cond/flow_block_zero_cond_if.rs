use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::CondChain;
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::asm_meta_helper::AssignMetaIfPool;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// Master ZIF branch — owns the chained zelif / zelse `con_blocks`.
/// Runs both the per-branch GRP pass on its own basic nodes and the
/// cross-branch merge across all continuation blocks.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroCondIf {
    base             : FlowBlockBase,
    cond_i           : HcpIdent,
    grp_asms         : Vec<AssignMeta>, ///// the assignment for the group
    result_basic_node: Option<NcpIdent>,
}

impl Default for FlowBlockZeroCondIf {
    fn default() -> Self { Self::new("", HcpIdent::default()) }
}

impl FlowBlockZeroCondIf {
    pub fn new(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base             : FlowBlockBase::new(FlowBlockType::ZeroCondIf, FlowBlockJoinPolicy::BasicNodeFlow, name),
            cond_i           : cond_i,
            grp_asms         : Vec::new(),
            result_basic_node: None,
        }
    }
}

impl FlowBlock for FlowBlockZeroCondIf {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        self.base.add_basic_node(_node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert_eq!(block.get_join_policy(), FlowBlockJoinPolicy::BasicNodeFlow,
            "zero-cond-if sub blocks must have BasicNodeFlow join policy");
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, block: FlowBlockIdent) {
        assert_eq!(block.get_block_type(), FlowBlockType::ZeroCondElif,
            "zero-cond-if con blocks must be ZeroCondElif");
        self.base.add_con_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_cond_if(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        // Per-branch GRP pass on the master's own basic nodes.
        self.grp_asms = self.base.gen_unified_asm_meta_flat(arena);

        // Cross-branch merge via AssignMetaIfPool.

        // master chain block
        let id        = self.base.get_ident().get_global_id();
        let self_cond = self.cond_i;
        let mut pool  = AssignMetaIfPool::default();
        pool.insert_asms(arena, Some(self_cond), self_cond, &self.grp_asms);

        // prev_false = ~self_cond
        let mut chain = CondChain::new_inv(arena, "zcif", id, self_cond);

        // cond block join
        let con_blocks_i: Vec<FlowBlockIdent> = self.base.get_con_blocks_i().to_vec();
        for (i, con_block_i) in con_blocks_i.into_iter().enumerate() {
            let con = arena.take_flow_block_zero_cond_elif(con_block_i);
            let con_cond = con.get_condition();

            let con_cond_abs = chain.step(arena, "zcif", id, i, con_cond).unwrap_or_default();

            pool.insert_asms(arena, con_cond, con_cond_abs, con.get_grp_asms());
            arena.replace_back_flow_block_zero_cond_elif(con);
        }

        // convert result asms to a single asm node
        let result_asms = pool.gen_assign_metas(arena);
        self.result_basic_node = Some(arena.make_asm_node_many(
            &format!("zcif_result_{}", id),
            &result_asms,
        ));
    }

    /// block summarization
    fn summarize_as_node(&self) -> NcpIdent { self.result_basic_node.expect("it has not generated yet")}
}

impl Identifiable for FlowBlockZeroCondIf {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
