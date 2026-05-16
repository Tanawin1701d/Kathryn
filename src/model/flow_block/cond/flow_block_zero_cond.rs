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

/// Zero-cycle conditional block covering ZIF (Some cond, no prior branch),
/// ZELIF (Some cond, chained branch), and ZELSE (None).
/// Hardware build is not implemented here; these are extracted at a higher
/// controller level before any hardware generation occurs.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroCond {
    base            : FlowBlockBase,
    condition       : Option<HcpIdent>,
    master_of_chain : bool,
    grp_asms        : Vec<AssignMeta>
}

impl Default for FlowBlockZeroCond {
    fn default() -> Self { Self::new_zelse("") }
}

impl FlowBlockZeroCond {
    pub fn new_zif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base           :      FlowBlockBase::new(FlowBlockType::ZeroCond, FlowBlockJoinPolicy::ExtFlow, name),
            condition      : Some(cond_i),
            master_of_chain: true,
            grp_asms       : Vec::new(),
        }
    }

    pub fn new_zelif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base           : FlowBlockBase::new(FlowBlockType::ZeroCond, FlowBlockJoinPolicy::ConFlow, name),
            condition      : Some(cond_i),
            master_of_chain: false,
            grp_asms       : Vec::new(),
        }
    }

    pub fn new_zelse(name: &str) -> Self {
        Self {
            base           : FlowBlockBase::new(FlowBlockType::ZeroCond, FlowBlockJoinPolicy::ConFlow, name),
            condition      : None,
            master_of_chain: false,
            grp_asms       : Vec::new(),
        }
    }
}

impl FlowBlock for FlowBlockZeroCond {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("zero-cond block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_con_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_cond(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        // Per-branch GRP pass: merge basic-node assign metas within this block.
        self.grp_asms = self.base.gen_unified_asm_meta_from_all_basic_nodes(arena);
        if !self.master_of_chain { return; }

        // Master (zif) only: cross-branch merge via AssignMetaIfPool.
        let id        = self.base.get_ident().get_global_id();
        let self_cond = self.condition.expect("master zif block must have a condition");
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
            let mut con = arena.take_flow_block_zero_cond(con_block_i);
            assert!(!con.master_of_chain, "con_block must be a zelif/zelse (master_of_chain=false)");
            let con_cond = con.condition;

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

            pool.insert_asms(arena, con_cond, con_cond_abs, &con.grp_asms);
            arena.replace_back_flow_block_zero_cond(con);
        }

        self.grp_asms = pool.gen_assign_metas(arena);
    }

    fn summarize_block(&self) -> NodeWrap {
        todo!("ZIF/ZELIF/ZELSE summarize not implemented — requires controller-level extraction before synthesis")
    }

    fn get_con_condition(&self) -> Option<HcpIdent> {
        self.condition
    }

    fn get_extract_node(&self) -> NcpIdent {
        todo!("get_unified_node for ZeroCond — requires controller-level extraction")
    }

}

impl Identifiable for FlowBlockZeroCond {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
