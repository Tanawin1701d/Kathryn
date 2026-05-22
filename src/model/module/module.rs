use std::collections::{HashMap, HashSet};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType, HW_TYPES_WITH_UE};
use crate::model::model_arena::{ModelArena, ModuleInitStage};
use crate::model::module::module_ident::ModuleIdent;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

// ---------------------------------------------------------------------------
// Module — container for a hardware module's user-declared components and
// implicit (design-flow built) internal_hws.  Stored in ModelArena.modules and
// referenced through ModuleIdent.  Mirrors C++ kathryn::Module, scoped to
// the subset of components currently ported to Rust.
// ---------------------------------------------------------------------------
#[derive(Default)]
pub struct Module {
    ident                : ModuleIdent,
    is_top_module        : bool,
    // start node
    start_node_i         : Option<NcpIdent>,
    // implicit internal_hws (built by the design flow); indexed by HwComponentType
    internal_hws         : [Vec<HcpIdent>; HwComponentType::COUNT],
    // user-declared hw components; indexed by HwComponentType
    user_hws             : [Vec<HcpIdent>; HwComponentType::COUNT],
    // user-declared sub module components
    user_sub_modules     : Vec<ModuleIdent>,
    // top flow blocks
    top_flow_blocks_i    : Vec<FlowBlockIdent>,
    // basic nodes
    basic_nodes_i        : Vec<NcpIdent>,

}

impl Module {
    pub fn new(is_user_com: bool, is_top_module: bool, name: &str) -> Self {
        Self {
            ident        : ModuleIdent::new(is_user_com, name),
            is_top_module,
            ..Default::default()
        }
    }

    pub fn mk(name: &str) -> Self { Self::new(true, false, name) }

    pub fn get_ident    (&self)              -> ModuleIdent { self.ident }
    pub fn set_ident    (&mut self, i: ModuleIdent)        { self.ident = i; }

    // -- top module flag --
    pub fn is_top_module(&self) -> bool { self.is_top_module }

    // -- sp_reg accessors (implicit, indexed by HwComponentType) --
    pub fn add_internal_hw(&mut self, i: HcpIdent) {
        let t = i.get_hw_type();
        self.internal_hws[t as usize].push(i);
    }
    pub fn get_internal_hws(&self, t: HwComponentType) -> &Vec<HcpIdent> {
        &self.internal_hws[t as usize]
    }

    // -- user-component accessors (explicit, indexed by HwComponentType) --
    pub fn add_user_hws(&mut self, i: HcpIdent) {
        let t = i.get_hw_type();
        self.user_hws[t as usize].push(i);
    }
    pub fn get_user_hws(&self, t: HwComponentType) -> &Vec<HcpIdent> {
        &self.user_hws[t as usize]
    }

    // -- sub-modules --
    pub fn add_user_sub_module(&mut self, i: ModuleIdent)   { self.user_sub_modules.push(i); }
    pub fn get_user_sub_modules(&self) -> &Vec<ModuleIdent> { &self.user_sub_modules }

    // -- start node --
    pub fn set_start_node(&mut self, i: NcpIdent)       { self.start_node_i = Some(i); }
    pub fn get_start_node(&self)  -> Option<NcpIdent>   { self.start_node_i }

    // -- top flow blocks --
    pub fn add_top_flow_block(&mut self, i: FlowBlockIdent)    { self.top_flow_blocks_i.push(i); }
    pub fn get_top_flow_blocks_i(&self) -> &Vec<FlowBlockIdent> { &self.top_flow_blocks_i }

    // -- basic nodes --
    pub fn add_basic_node(&mut self, i: NcpIdent)       { self.basic_nodes_i.push(i); }
    pub fn get_basic_nodes_i(&self) -> &Vec<NcpIdent>   { &self.basic_nodes_i }

    // -- dependency gathering --

    /// Collect all HcpIdents depended on by every HCP registered in this module
    /// (both internal and user), traversing the full UpdateEvent tree of each pool.
    /// `self` must already be taken out of the arena so arena is free for HCP access.
    pub fn gather_dep_hcps(&self, arena: &mut ModelArena, out: &mut HashSet<HcpIdent>) {
        for &hw_type in &HW_TYPES_WITH_UE {
            for &hcp_i in self.get_internal_hws(hw_type).iter()
                              .chain(self.get_user_hws(hw_type).iter())
            {
                let hcp = arena.take_hcp(hcp_i);
                hcp.gather_dep_hcps(arena, out);
                arena.replace_back_hcp(hcp);
            }
        }
    }

    /// Rewrite every HcpIdent dependency in this module's UpdateEvents according to `map`.
    /// `self` must already be taken out of the arena so arena is free for HCP access.
    pub fn remap_dep_hcps(&self, map: &HashMap<HcpIdent, HcpIdent>, arena: &mut ModelArena) {
        for &hw_type in &HW_TYPES_WITH_UE {
            for &hcp_i in self.get_internal_hws(hw_type).iter()
                              .chain(self.get_user_hws(hw_type).iter())
            {
                let mut hcp = arena.take_hcp(hcp_i);
                hcp.remap_dep_hcps(map, arena);
                arena.replace_back_hcp(hcp);
            }
        }
    }

    // -- build --

    /// Build all flow blocks and nodes for a top-level module.
    /// Uses `start_node_i` stored on `self`.
    pub fn build_flow_as_top_module(&mut self, arena: &mut ModelArena) {
        let start_node_i = self.start_node_i.expect("top module must have a start node set before build_flow");
        self.build_flow_base(arena, start_node_i);
    }

    /// Build all flow blocks and nodes for a sub-module.
    /// `start_node_i` is provided by the parent (not stored on self).
    pub fn build_flow_as_sub_module(&mut self, arena: &mut ModelArena, start_node_i: NcpIdent) {
        self.build_flow_base(arena, start_node_i);
    }

    fn build_flow_base(&mut self, arena: &mut ModelArena, start_node_i: NcpIdent) {
        // push this module to flow stack
        arena.push_module_trace_stack(self.ident, ModuleInitStage::FlowBlockBuild);

        // Top flow blocks: build, then wire or assign depending on join policy.
        for &block_i in &self.top_flow_blocks_i {
            arena.build_flow_block(block_i);
            if block_i.get_join_policy() == FlowBlockJoinPolicy::BasicNodeFlow {
                // BasicNodeFlow blocks expose a single asm node — dry-assign it.
                let node_i = arena.get_flow_block(block_i).summarize_as_node();
                assert_eq!(node_i.get_node_type(), NodeType::Asm,
                    "BasicNodeFlow summarize_as_node must return an AsmNode");
                arena.dry_assign_asm_node(node_i);
            } else {
                // SubFlow blocks expose a NodeWrap — wire entrances to start_node then assign.
                let wrap = arena.summarize_flow_block(block_i);
                wrap.add_dep_to_entrances(arena, start_node_i, None);
                wrap.assign_entrance_nodes(arena);
            }
        }

        // Basic nodes: dry-assign only, no start node dependency.
        for &node_i in &self.basic_nodes_i {
            assert_eq!(node_i.get_node_type(), NodeType::Asm,
                "basic_nodes_i must contain only AsmNodes");
            arena.dry_assign_asm_node(node_i);
        }

        // pop this module from flow stack
        arena.pop_module_trace_stack();

        // register HCPs that were created during FlowBlockBuild, we do this because ownership issue
        for (hcp_i, is_user) in arena.drain_hcp_pending_buffer() {
            if is_user { self.add_user_hws(hcp_i); } else { self.add_internal_hw(hcp_i); }
        }

        // Sub-modules: pass start_node_i down.
        let sub_module_ids: Vec<ModuleIdent> = self.user_sub_modules.clone();
        for sub_module_i in sub_module_ids {
            let mut sub_module = arena.take_module(sub_module_i);
            sub_module.build_flow_as_sub_module(arena, start_node_i);
            arena.replace_back_module(sub_module_i, sub_module);
        }
    }
}

impl Identifiable for Module {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
