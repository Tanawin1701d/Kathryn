use std::collections::{HashMap, HashSet};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy};
use crate::model::flow_block::ExtSigType;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType, HW_TYPES_WITH_UE, HW_TYPES_WITH_MAN_DEP, ALL_HW_TYPES};
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
    // complex component properties (CCPs) stamped into this module
    ccps_i               : Vec<CcpIdent>,
    // top flow blocks
    top_flow_blocks_i    : Vec<FlowBlockIdent>,
    // basic nodes
    basic_nodes_i        : Vec<NcpIdent>,

}

impl Module {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        Self {
            ident        : ModuleIdent::new(is_user_com, name),
            ..Default::default()   // is_top_module defaults to false; set via set_is_top_module
        }
    }

    pub fn get_ident    (&self)             -> ModuleIdent { self.ident }
    pub fn set_ident    (&mut self, i: ModuleIdent)        { self.ident = i; }

    // -- top module flag --
    pub fn is_top_module    (&self)               -> bool { self.is_top_module }
    pub fn set_is_top_module(&mut self, v: bool)          { self.is_top_module = v; }

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

    // -- complex component properties (CCPs) --
    pub fn add_ccp(&mut self, i: CcpIdent)        { self.ccps_i.push(i); }
    pub fn get_ccps(&self) -> &Vec<CcpIdent>      { &self.ccps_i }

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
        for &hw_type in HW_TYPES_WITH_UE.iter().chain(HW_TYPES_WITH_MAN_DEP.iter()) {
            for &hcp_i in self.get_internal_hws(hw_type).iter()
                              .chain(self.get_user_hws(hw_type).iter())
            {
                let hcp = arena.take_hcp(hcp_i);
                hcp.gather_dep_hcps(arena, out);
                arena.replace_back_hcp(hcp);
            }
        }

    }

    /// Collect every HCP in this module (internal + user) that carries an IO mark.
    /// `self` must already be taken out of the arena so arena is free for HCP access.
    pub fn gather_io_marked_hcps(&self, arena: &ModelArena, out: &mut Vec<HcpIdent>) {
        for hw_type in ALL_HW_TYPES {
            for &hcp_i in self.get_internal_hws(hw_type).iter()
                              .chain(self.get_user_hws(hw_type).iter())
            {
                if arena.get_io_mark(&hcp_i).is_some() {
                    out.push(hcp_i);
                }
            }
        }
    }

    /// Sort the UpdatePool of every HCP in this module by priority then sub-priority.
    /// `self` must already be taken out of the arena so arena is free for HCP access.
    pub fn sort_update_event_pool(&self, arena: &mut ModelArena) {
        for &hw_type in &HW_TYPES_WITH_UE {
            for &hcp_i in self.get_internal_hws(hw_type).iter()
                              .chain(self.get_user_hws(hw_type).iter())
            {
                let mut hcp = arena.take_hcp(hcp_i);
                hcp.sort_events(arena);
                arena.replace_back_hcp(hcp);
            }
        }
    }

    /// Rewrite every HcpIdent dependency in this module's UpdateEvents according to `map`.
    /// `self` must already be taken out of the arena so arena is free for HCP access.
    pub fn remap_dep_hcps(&self, map: &HashMap<HcpIdent, HcpIdent>, arena: &mut ModelArena) {
        // Must cover the same type set as gather_dep_hcps — manual-dep types
        // (Expression) carry operand handles that need remapping too.
        for &hw_type in HW_TYPES_WITH_UE.iter().chain(HW_TYPES_WITH_MAN_DEP.iter()) {
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

    /// Build all flow blocks and nodes for a top-level module. The top module
    /// owns the primitive `clk` / master-reset inputs and the start node; both
    /// reset signal and clock are forwarded down to every sub-module from here.
    pub fn build_flow_as_top_module(&mut self, arena: &mut ModelArena) {
        // The start node is built here, not supplied — it must not exist yet.
        assert!(self.start_node_i.is_none(),
                "top module start node must not be set before build_flow");

        // Create the top-level HCPs in this module's FlowBlockBuild scope so they
        // land in the pending buffer and get registered by build_flow_base's drain.
        arena.push_module_trace_stack(self.ident, ModuleInitStage::FlowBlockBuild);

        // clk + master-reset wires, marked as top-level primitive inputs. They are
        // driven externally, so they carry no fallback default (not even zero).
        let clk_i = arena.make_wire(false, "clk", 1);
        arena.mark_as_io(clk_i, true, "clk".to_string());
        arena.disable_wire_default(clk_i);
        let mreset_i = arena.make_wire(false, "mrst", 1);
        arena.mark_as_io(mreset_i, true, "mrst".to_string());
        arena.disable_wire_default(mreset_i);

        // Start node, reset by the master-reset wire.
        let start_node_i = arena.make_start_node("start", mreset_i);
        // Wire the start node's clock to the same top-level clk the flow blocks
        // receive, so its state reg shares one clock net (else its clk is never set).
        let clk_node_i = arena.make_opr_node("start_clk", clk_i);
        arena.set_ncp_clk_node(start_node_i, clk_node_i);
        arena.assign_ncp_node(start_node_i, true, true);
        self.set_start_node(start_node_i);

        arena.pop_module_trace_stack();

        self.build_flow_base(arena, start_node_i, clk_i, mreset_i);
    }

    /// Build all flow blocks and nodes for a sub-module. `start_node_i`, `clk_i`
    /// and `mreset_i` are forwarded by the parent (not owned by this module).
    pub fn build_flow_as_sub_module(
        &mut self,
        arena       : &mut ModelArena,
        start_node_i: NcpIdent,
        clk_i       : HcpIdent,
        mreset_i    : HcpIdent,
    ) {
        self.build_flow_base(arena, start_node_i, clk_i, mreset_i);
    }

    /// Drain HCPs buffered during FlowBlockBuild (deferred because the module was
    /// taken out of the arena while building) and file each into the right list.
    fn register_pending_hcps(&mut self, arena: &mut ModelArena) {
        for (hcp_i, is_user) in arena.drain_hcp_pending_buffer() {
            if is_user { self.add_user_hws(hcp_i); } else { self.add_internal_hw(hcp_i); }
        }
    }

    /// Build the user-declared reset events for every reg and default events for
    /// every wire registered in this module. Reg resets clock off the module clk
    /// (`clk_i`); wire defaults are combinational, low-priority fallbacks. Both are
    /// no-ops on HCPs the user never gave a reset / default value to.
    fn build_reset_and_default_events(&self, arena: &mut ModelArena, clk_i: HcpIdent, mreset_i: HcpIdent) {
        for &reg_i in self.get_internal_hws(HwComponentType::Reg).iter()
                          .chain(self.get_user_hws(HwComponentType::Reg).iter())
        {
            let mut reg = arena.take_reg(reg_i);
            reg.try_build_reset(clk_i, mreset_i, arena);
            arena.replace_back_reg(reg);
        }
        for &wire_i in self.get_internal_hws(HwComponentType::Wire).iter()
                           .chain(self.get_user_hws(HwComponentType::Wire).iter())
        {
            let mut wire = arena.take_wire(wire_i);
            wire.try_build_default(arena);
            arena.replace_back_wire(wire);
        }
    }

    fn build_flow_base(
        &mut self,
        arena       : &mut ModelArena,
        start_node_i: NcpIdent,
        clk_i       : HcpIdent,
        mreset_i    : HcpIdent,
    ) {
        // push this module to flow stack
        arena.push_module_trace_stack(self.ident, ModuleInitStage::FlowBlockBuild);

        // Top flow blocks: build, then wire or assign depending on join policy.
        for &block_i in &self.top_flow_blocks_i {
            // Forward the module-wide clk / master-reset as int signals before the
            // build so build_common_hw wires them and propagates to nested blocks.
            arena.add_ext_signal_to_flow_block(block_i, ExtSigType::Clk, clk_i);
            arena.add_ext_signal_to_flow_block(block_i, ExtSigType::MReset, mreset_i);

            arena.build_flow_block(block_i);
            if block_i.get_join_policy() == FlowBlockJoinPolicy::BasicNodeFlow {
                // BasicNodeFlow blocks expose a single asm node — dry-assign it.
                let block  = arena.take_flow_block(block_i);
                let node_i = block.summarize_as_node();
                arena.replace_back_flow_block(block);
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
            // Fold each meta's deferred pending pre-condition before the final update.
            arena.apply_asm_node_pending_pre_cond(node_i);
            arena.dry_assign_asm_node(node_i);
        }

        // Build every CCP stamped into this module while still in the FlowBlockBuild
        // scope, so the expressions each build creates land in the HCP pending buffer
        // (drained below). CCPs are only ever created during the construction phase,
        // so the list is final here.
        let ccp_ids: Vec<CcpIdent> = self.ccps_i.clone();
        for ccp_i in ccp_ids {
            arena.build_ccp(ccp_i);
        }

        // pop this module from flow stack
        arena.pop_module_trace_stack();
        // register HCPs buffered during the flow build (deferred for ownership reasons)
        self.register_pending_hcps(arena);


        // Build the user reset (regs) / default (wires) fallback events now that
        arena.push_module_trace_stack(self.ident, ModuleInitStage::FlowBlockBuild);
        // every HCP of this module is registered and the module clk is known.
        self.build_reset_and_default_events(arena, clk_i, mreset_i);
        arena.pop_module_trace_stack();
        // register HCPs buffered during the flow build (deferred for ownership reasons)
        self.register_pending_hcps(arena);





        // Sub-modules: send the start / clk / master-reset signals down to slaves.
        let sub_module_ids: Vec<ModuleIdent> = self.user_sub_modules.clone();
        for sub_module_i in sub_module_ids {
            let mut sub_module = arena.take_module(sub_module_i);
            sub_module.build_flow_as_sub_module(arena, start_node_i, clk_i, mreset_i);
            arena.replace_back_module(sub_module_i, sub_module);
        }
    }
}

impl Identifiable for Module {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}
