use crate::common::arena_base::ArenaGroup;
use crate::model::common::identifier::Identifiable;
use crate::model::model_arena::{ModelArena, ModuleInitStage};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::module::module::Module;
use crate::model::module::module_ident::ModuleIdent;
use crate::model::flow_block::{FlowBlockIdent, FlowBlockJoinPolicy};

// ---------------------------------------------------------------------------
// ModelArena::new / reset live here. Per-category CRUD lives in:
//   - arena_impl_hwc.rs   (hardware components: Reg/Wire/Val/.../sp_regs)
//   - arena_impl_ue.rs    (update events)
//   - arena_impl_node.rs  (flow nodes)
// Module CRUD lives at the bottom of this file.
// ---------------------------------------------------------------------------

impl ModelArena {
    pub fn new() -> Self {
        Self {
            regs       : ArenaGroup::new(),
            wires      : ArenaGroup::new(),
            io_wires   : ArenaGroup::new(),
            vals       : ArenaGroup::new(),
            mem_eles   : ArenaGroup::new(),
            mem_blks   : ArenaGroup::new(),
            expressions: ArenaGroup::new(),
            state_regs : ArenaGroup::new(),
            sync_regs  : ArenaGroup::new(),
            cnt_regs   : ArenaGroup::new(),
            cond_wait_regs : ArenaGroup::new(),
            cycle_wait_regs: ArenaGroup::new(),
            ue_basics  : ArenaGroup::new(),
            ue_grps    : ArenaGroup::new(),
            ue_conds   : ArenaGroup::new(),
            ue_switches: ArenaGroup::new(),
            asm_nodes        : ArenaGroup::new(),
            start_nodes      : ArenaGroup::new(),
            state_nodes      : ArenaGroup::new(),
            syn_nodes        : ArenaGroup::new(),
            wait_cond_nodes  : ArenaGroup::new(),
            wait_cycle_nodes : ArenaGroup::new(),
            counter_nodes    : ArenaGroup::new(),
            pseudo_nodes     : ArenaGroup::new(),
            opr_nodes        : ArenaGroup::new(),
            modules          : ArenaGroup::new(),
            top_module              : None,
            module_trace_stack      : Vec::new(),
            hcp_pending_buffer      : Vec::new(),
            flow_block_init_stack   : Vec::new(),
            flow_block_seqs           : ArenaGroup::new(),
            flow_block_pars           : ArenaGroup::new(),
            flow_block_conds          : ArenaGroup::new(),
            flow_block_cond_elifs     : ArenaGroup::new(),
            flow_block_zero_cond_ifs  : ArenaGroup::new(),
            flow_block_zero_cond_elifs: ArenaGroup::new(),
            flow_block_zero_switches    : ArenaGroup::new(),
            flow_block_zero_switch_cases: ArenaGroup::new(),
            flow_block_whiles         : ArenaGroup::new(),
            flow_block_do_whiles      : ArenaGroup::new(),
            flow_block_counter_loops  : ArenaGroup::new(),
        }
    }

    pub fn reset(&mut self) {
        self.regs        = ArenaGroup::new();
        self.wires       = ArenaGroup::new();
        self.io_wires    = ArenaGroup::new();
        self.vals        = ArenaGroup::new();
        self.mem_eles    = ArenaGroup::new();
        self.mem_blks    = ArenaGroup::new();
        self.expressions = ArenaGroup::new();
        self.state_regs  = ArenaGroup::new();
        self.sync_regs   = ArenaGroup::new();
        self.cnt_regs    = ArenaGroup::new();
        self.cond_wait_regs  = ArenaGroup::new();
        self.cycle_wait_regs = ArenaGroup::new();
        self.ue_basics   = ArenaGroup::new();
        self.ue_grps     = ArenaGroup::new();
        self.ue_conds    = ArenaGroup::new();
        self.ue_switches = ArenaGroup::new();
        self.asm_nodes        = ArenaGroup::new();
        self.start_nodes      = ArenaGroup::new();
        self.state_nodes      = ArenaGroup::new();
        self.syn_nodes        = ArenaGroup::new();
        self.wait_cond_nodes  = ArenaGroup::new();
        self.wait_cycle_nodes = ArenaGroup::new();
        self.counter_nodes    = ArenaGroup::new();
        self.pseudo_nodes     = ArenaGroup::new();
        self.opr_nodes        = ArenaGroup::new();
        self.modules          = ArenaGroup::new();
        self.top_module             = None;
        self.module_trace_stack     = Vec::new();
        self.hcp_pending_buffer     = Vec::new();
        self.flow_block_init_stack  = Vec::new();
        self.flow_block_seqs           = ArenaGroup::new();
        self.flow_block_pars           = ArenaGroup::new();
        self.flow_block_conds          = ArenaGroup::new();
        self.flow_block_cond_elifs     = ArenaGroup::new();
        self.flow_block_zero_cond_ifs   = ArenaGroup::new();
        self.flow_block_zero_cond_elifs = ArenaGroup::new();
        self.flow_block_zero_switches     = ArenaGroup::new();
        self.flow_block_zero_switch_cases = ArenaGroup::new();
        self.flow_block_whiles         = ArenaGroup::new();
        self.flow_block_do_whiles      = ArenaGroup::new();
        self.flow_block_counter_loops  = ArenaGroup::new();
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

    // -----------------------------------------------------------------------
    // Top module
    // -----------------------------------------------------------------------
    pub fn set_top_module(&mut self, i: ModuleIdent) {
        assert!(self.top_module.is_none(), "top_module is already set");
        self.top_module = Some(i);
    }
    pub fn get_top_module(&self) -> Option<ModuleIdent> { self.top_module }

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
    pub fn push_flow_block_init_stack(&mut self, i: FlowBlockIdent) { self.flow_block_init_stack.push(i); }
    pub fn pop_flow_block_init_stack (&mut self) -> FlowBlockIdent  { self.flow_block_init_stack.pop().expect("flow block stack is empty") }
    pub fn peek_flow_block_init_stack(&self)     -> FlowBlockIdent  { *self.flow_block_init_stack.last().expect("flow block stack is empty") }

    /// Pop the top flow block, assert it matches `expected`, then attach it:
    /// - to the new stack top as a sub-flow-block, if the stack is non-empty, or
    /// - to the module on the trace stack (must be in FlowBlockInit stage) as a top flow block.
    pub fn finalize_flow_block(&mut self, expected: FlowBlockIdent) {
        let popped = self.pop_flow_block_init_stack();
        assert_eq!(popped, expected, "finalize_flow_block: ident mismatch — wrong block finalized");

        if let Some(&parent_i) = self.flow_block_init_stack.last() {
            match popped.get_join_policy() {
                FlowBlockJoinPolicy::SubFlow | FlowBlockJoinPolicy::BasicNodeFlow =>
                    self.add_sub_flow_block_to_flow_block(parent_i, popped),
                FlowBlockJoinPolicy::ConFlow =>
                    self.add_con_flow_block_to_flow_block(parent_i, popped),
            }
        } else {
            let (module_i, stage) = self.peek_module_trace_stack();
            assert_eq!(stage, ModuleInitStage::FlowBlockInit,
                "finalize_flow_block: expected FlowBlockInit stage on module trace stack, got {:?}", stage);
            let mut m = self.take_module(module_i);
            m.add_top_flow_block(popped);
            self.replace_back_module(module_i, m);
        }
    }
}
