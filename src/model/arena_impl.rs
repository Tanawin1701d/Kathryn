use crate::common::arena_base::ArenaGroup;
use crate::model::common::identifier::Identifiable;
use crate::model::model_arena::{ModelArena, ModuleInitStage};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::module::module::Module;
use crate::model::module::module_ident::ModuleIdent;
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::model::flow_block::{BlockTrackStatus, FlowBlock, FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::FlowBlockJoinPolicy::{ConFlow, SubFlow};
// ---------------------------------------------------------------------------
// ModelArena::new / reset live here. Per-category CRUD lives in:
//   - arena_impl_hwc.rs        (hardware components: Reg/Wire/Val/.../sp_regs)
//   - arena_impl_ue.rs         (update events)
//   - arena_impl_node.rs       (flow nodes)
//   - arena_impl_flow_block.rs (flow blocks)
// Module CRUD lives at the bottom of this file.
// ---------------------------------------------------------------------------

impl ModelArena {
    pub fn new() -> Self {
        Self {
            // ---- hardware components ----
            regs                         : ArenaGroup::new(),
            wires                        : ArenaGroup::new(),
            io_wires                     : ArenaGroup::new(),
            vals                         : ArenaGroup::new(),
            mem_eles                     : ArenaGroup::new(),
            mem_blks                     : ArenaGroup::new(),
            expressions                  : ArenaGroup::new(),
            state_regs                   : ArenaGroup::new(),
            sync_regs                    : ArenaGroup::new(),
            cnt_regs                     : ArenaGroup::new(),
            cond_wait_regs               : ArenaGroup::new(),
            cycle_wait_regs              : ArenaGroup::new(),
            // ---- update events ----
            ue_basics                    : ArenaGroup::new(),
            ue_grps                      : ArenaGroup::new(),
            ue_conds                     : ArenaGroup::new(),
            ue_switches                  : ArenaGroup::new(),
            // ---- flow nodes ----
            asm_nodes                    : ArenaGroup::new(),
            start_nodes                  : ArenaGroup::new(),
            state_nodes                  : ArenaGroup::new(),
            syn_nodes                    : ArenaGroup::new(),
            wait_cond_nodes              : ArenaGroup::new(),
            wait_cycle_nodes             : ArenaGroup::new(),
            counter_nodes                : ArenaGroup::new(),
            pseudo_nodes                 : ArenaGroup::new(),
            opr_nodes                    : ArenaGroup::new(),
            // ---- complex hardware (CCP) ----
            arbs                         : ArenaGroup::new(),
            karrays                      : ArenaGroup::new(),
            // ---- modules & trace state ----
            modules                      : ArenaGroup::new(),
            top_module                   : None,
            module_trace_stack           : Vec::new(),
            hcp_pending_buffer           : Vec::new(),
            flow_block_init_stack        : Vec::new(),
            // ---- flow blocks ----
            flow_block_seqs              : ArenaGroup::new(),
            flow_block_pars              : ArenaGroup::new(),
            flow_block_conds             : ArenaGroup::new(),
            flow_block_cond_elifs        : ArenaGroup::new(),
            flow_block_zero_cond_ifs     : ArenaGroup::new(),
            flow_block_zero_cond_elifs   : ArenaGroup::new(),
            flow_block_zero_switches     : ArenaGroup::new(),
            flow_block_zero_switch_cases : ArenaGroup::new(),
            flow_block_picks             : ArenaGroup::new(),
            flow_block_pick_ifs          : ArenaGroup::new(),
            flow_block_whiles            : ArenaGroup::new(),
            flow_block_do_whiles         : ArenaGroup::new(),
            flow_block_counter_loops     : ArenaGroup::new(),
            flow_block_waits             : ArenaGroup::new(),
            flow_block_pips              : ArenaGroup::new(),
            flow_block_zyncs             : ArenaGroup::new(),
        }
    }

    pub fn reset(&mut self) {
        // ---- hardware components ----
        self.regs                         = ArenaGroup::new();
        self.wires                        = ArenaGroup::new();
        self.io_wires                     = ArenaGroup::new();
        self.vals                         = ArenaGroup::new();
        self.mem_eles                     = ArenaGroup::new();
        self.mem_blks                     = ArenaGroup::new();
        self.expressions                  = ArenaGroup::new();
        self.state_regs                   = ArenaGroup::new();
        self.sync_regs                    = ArenaGroup::new();
        self.cnt_regs                     = ArenaGroup::new();
        self.cond_wait_regs               = ArenaGroup::new();
        self.cycle_wait_regs              = ArenaGroup::new();
        // ---- update events ----
        self.ue_basics                    = ArenaGroup::new();
        self.ue_grps                      = ArenaGroup::new();
        self.ue_conds                     = ArenaGroup::new();
        self.ue_switches                  = ArenaGroup::new();
        // ---- flow nodes ----
        self.asm_nodes                    = ArenaGroup::new();
        self.start_nodes                  = ArenaGroup::new();
        self.state_nodes                  = ArenaGroup::new();
        self.syn_nodes                    = ArenaGroup::new();
        self.wait_cond_nodes              = ArenaGroup::new();
        self.wait_cycle_nodes             = ArenaGroup::new();
        self.counter_nodes                = ArenaGroup::new();
        self.pseudo_nodes                 = ArenaGroup::new();
        self.opr_nodes                    = ArenaGroup::new();
        // ---- complex hardware (CCP) ----
        self.arbs                         = ArenaGroup::new();
        self.karrays                      = ArenaGroup::new();
        // ---- modules & trace state ----
        self.modules                      = ArenaGroup::new();
        self.top_module                   = None;
        self.module_trace_stack           = Vec::new();
        self.hcp_pending_buffer           = Vec::new();
        self.flow_block_init_stack        = Vec::new();
        // ---- flow blocks ----
        self.flow_block_seqs              = ArenaGroup::new();
        self.flow_block_pars              = ArenaGroup::new();
        self.flow_block_conds             = ArenaGroup::new();
        self.flow_block_cond_elifs        = ArenaGroup::new();
        self.flow_block_zero_cond_ifs     = ArenaGroup::new();
        self.flow_block_zero_cond_elifs   = ArenaGroup::new();
        self.flow_block_zero_switches     = ArenaGroup::new();
        self.flow_block_zero_switch_cases = ArenaGroup::new();
        self.flow_block_picks             = ArenaGroup::new();
        self.flow_block_pick_ifs          = ArenaGroup::new();
        self.flow_block_whiles            = ArenaGroup::new();
        self.flow_block_do_whiles         = ArenaGroup::new();
        self.flow_block_counter_loops     = ArenaGroup::new();
        self.flow_block_waits             = ArenaGroup::new();
        self.flow_block_pips              = ArenaGroup::new();
        self.flow_block_zyncs             = ArenaGroup::new();
    }

    // -----------------------------------------------------------------------
    // Module insert / take / replace_back
    // -----------------------------------------------------------------------
    pub fn add_module(&mut self, m: Module) -> ModuleIdent {
        let h = self.modules.insert(m);
        self.modules.get(h).get_ident()
    }

    pub fn take_module        (&mut self, i: ModuleIdent)         -> Module { self.modules.take(*i.get_arena_handle()) }
    pub fn replace_back_module(&mut self, i: ModuleIdent, v: Module)        { self.modules.replace_back(*i.get_arena_handle(), v) }
    pub(crate) fn get_module_ident_by_handle(&self, h: crate::common::arena_base::ArenaHandle) -> ModuleIdent { self.modules.get(h).get_ident() }

    // -----------------------------------------------------------------------
    // Top module
    // -----------------------------------------------------------------------
    pub fn set_top_module(&mut self, i: ModuleIdent) {
        assert!(self.top_module.is_none(), "top_module is already set");
        self.top_module = Some(i);
        // Flag the module itself so module-side queries agree with the arena.
        let mut m = self.take_module(i);
        m.set_is_top_module(true);
        self.replace_back_module(i, m);
    }
    pub fn get_top_module(&self) -> Option<ModuleIdent> { self.top_module }

    /// Build the whole model starting from the top module's flow.
    pub fn build_flow(&mut self) {
        let top_i = self.top_module.expect("build_flow: no top module set");
        let mut top = self.take_module(top_i);
        top.build_flow_as_top_module(self);
        self.replace_back_module(top_i, top);
    }

    // -----------------------------------------------------------------------
    // Module trace stack — (ModuleIdent, ModuleInitStage) pairs
    // -----------------------------------------------------------------------
    pub fn push_module_trace_stack(&mut self, i: ModuleIdent, stage: ModuleInitStage) {
        self.module_trace_stack.push((i, stage));
    }
    pub fn pop_module_trace_stack(&mut self) -> (ModuleIdent, ModuleInitStage) {
        self.module_trace_stack.pop().expect("module trace stack is empty")
    }
    pub fn peek_module_trace_stack(&self) -> (ModuleIdent, ModuleInitStage) {
        *self.module_trace_stack.last().expect("module trace stack is empty")
    }
    pub fn try_peek_module_trace_stack(&self) -> Option<(ModuleIdent, ModuleInitStage)> {
        self.module_trace_stack.last().copied()
    }

    // -----------------------------------------------------------------------
    // HCP pending buffer — holds HCPs created during FlowBlockBuild stage;
    // the owning module drains and registers them itself.
    // -----------------------------------------------------------------------
    pub fn drain_hcp_pending_buffer(&mut self) -> Vec<(HcpIdent, bool)> {
        std::mem::take(&mut self.hcp_pending_buffer)
    }


    // -----------------------------------------------------------------------
    // Flow-block stack — tracks the active flow block during build traversal
    // -----------------------------------------------------------------------
    pub fn push_flow_block_init_stack(&mut self, i: FlowBlockIdent, status: BlockTrackStatus) { self.flow_block_init_stack.push((i, status)); }
    pub fn pop_flow_block_init_stack (&mut self) -> FlowBlockIdent  { self.flow_block_init_stack.pop().expect("flow block stack is empty").0 }
    pub fn peek_flow_block_init_stack(&self)     -> FlowBlockIdent  { self.flow_block_init_stack.last().expect("flow block stack is empty").0 }

    pub fn peek_flow_block_init_status(&self)    -> BlockTrackStatus { self.flow_block_init_stack.last().expect("flow block stack is empty").1 }

    /// Walk the init stack from the top down and return the type of the most
    /// recent skeleton block (Sequential or Parallel). Cond / loop blocks are
    /// skipped; None if the stack holds no seq/par block.
    pub fn get_last_skeleton_flow_block_type(&self) -> FlowBlockType {
        self.flow_block_init_stack
            .iter()
            .rev()
            .map        (|(b, _)| b.get_block_type())
            .find       (|t| matches!(t, FlowBlockType::Sequential | FlowBlockType::Parallel))
            .unwrap_or  (FlowBlockType::Parallel)
    }



    /// Validate a flow block against the constraints its `build` would otherwise
    /// assert (e.g. "cond block must have at least one body block"). Returns the
    /// constraint message as `Err` instead of panicking, so callers (notably the
    /// Python binding) can raise a precise, recoverable error before finalize.
    pub fn check_flow_block_prefinalize(&mut self, ident: FlowBlockIdent) -> Result<(), String> {
        let block = self.take_flow_block(ident);
        let res   = block.check_prefinalize();
        self.replace_back_flow_block(block);
        res
    }

    pub fn try_clean_lazy_closed_in_flow_block_stack(&mut self){
        // a lazy-closed chain master lingers on the stack only to catch a following
        // continuation branch; once the caller knows none will follow it retires it.
        if !self.flow_block_init_stack.is_empty()
            && self.peek_flow_block_init_status() == BlockTrackStatus::LazyClosed {
            self.finalize_flow_block(self.peek_flow_block_init_stack(), false);
        }
    }

    pub fn initialize_flow_block(&mut self, fb_ident: FlowBlockIdent) {

        if fb_ident.get_join_policy() != ConFlow {
            self.try_clean_lazy_closed_in_flow_block_stack()
        }
        self.push_flow_block_init_stack(fb_ident, BlockTrackStatus::OpenForSubBlock);

    }

    /// Pop the top flow block, assert it matches `expected`, then attach it:
    /// - to the new stack top as a sub-flow-block, if the stack is non-empty, or
    /// - to the module on the trace stack (must be in FlowBlockInit stage) as a top flow block.
    pub fn finalize_flow_block(&mut self, expected: FlowBlockIdent, is_recur: bool) {

        // The only legal mismatch is a single lazy-closed chain master lingering above
        // `expected` (its continuation chain ended without a trailing sibling). The model
        // constraint that a conditional always wraps a seq sub-block guarantees no two
        // chain masters are ever adjacent, so one recursion level always resolves it.
        if self.peek_flow_block_init_stack() != expected {
            assert!(!is_recur, "finalize_flow_block: recursive finalize must not mismatch the expected block");
            self.finalize_flow_block(self.peek_flow_block_init_stack(), true);
        }

        let popped_status = self.peek_flow_block_init_status();
        let popped_fb = self.pop_flow_block_init_stack();
        assert_eq!(popped_fb, expected, "finalize_flow_block: ident mismatch — wrong block finalized");

        // a chain master that is still open keeps lingering on the stack (now lazy-closed)
        // so a following continuation branch can attach to it; defer its real finalize.
        if popped_status == BlockTrackStatus::OpenForSubBlock && popped_fb.get_chain_master() {
            self.push_flow_block_init_stack(popped_fb, BlockTrackStatus::LazyClosed);
            return;
        }

        // right now it is not open for sub block or not chain master
        if let Some(&(parent_i, _)) = self.flow_block_init_stack.last() {
            match popped_fb.get_join_policy() {
                FlowBlockJoinPolicy::SubFlow | FlowBlockJoinPolicy::BasicNodeFlow =>
                    self.add_sub_flow_block_to_flow_block(parent_i, popped_fb),
                FlowBlockJoinPolicy::ConFlow =>
                    self.add_con_flow_block_to_flow_block(parent_i, popped_fb),
            }
        } else {
            let (module_i, stage) = self.peek_module_trace_stack();
            assert_eq!(stage, ModuleInitStage::FlowBlockInit,
                       "finalize_flow_block: expected FlowBlockInit stage on module trace stack, got {:?}", stage);
            let mut m = self.take_module(module_i);
            m.add_top_flow_block(popped_fb);
            self.replace_back_module(module_i, m);
        }
    }

    /// used when we want to finalize all flow in the module's flow
    pub fn finalize_flow_procedure(&mut self) {
        assert!(self.flow_block_init_stack.len() <= 1,
                "finalize_flow_procedure: flow block stack must have 0 or 1 element, got {}",
                self.flow_block_init_stack.len());
        // the only legal leftover is a lingering lazy-closed chain master; a still-open
        // block here means an unbalanced flow that try_clean would silently drop.
        assert!(self.flow_block_init_stack.is_empty()
                || self.peek_flow_block_init_status() == BlockTrackStatus::LazyClosed,
                "finalize_flow_procedure: leftover flow block must be lazy-closed, not still open");
        self.try_clean_lazy_closed_in_flow_block_stack();
    }


    /// Attach a basic node to wherever the build is currently pointing: the active
    /// flow block on top of the init stack, or the top module if no block is building.
    pub fn attach_basic_node_to_current_scope(&mut self, node_i: NcpIdent) {
        self.try_clean_lazy_closed_in_flow_block_stack();
        if let Some(&(block_i, _)) = self.flow_block_init_stack.last() {
            self.add_node_to_flow_block(block_i, node_i);
        } else {
            self.add_basic_node_to_top_module(node_i);
        }
    }

    /// Push a basic node onto the top module's basic-node list.
    pub fn add_basic_node_to_top_module(&mut self, node_i: NcpIdent) {
        let module_i = self.get_top_module().expect("add_basic_node_to_top_module: no top module set");
        let mut m = self.take_module(module_i);
        m.add_basic_node(node_i);
        self.replace_back_module(module_i, m);
    }
}
