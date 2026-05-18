use crate::common::arena_base::ArenaGroup;
use crate::model::common::identifier::Identifiable;
use crate::model::model_arena::ModelArena;
use crate::model::module::module::Module;
use crate::model::module::module_ident::ModuleIdent;

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
            module_comp_init_stack: Vec::new(),
            module_flow_init_track: None,
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
        self.module_comp_init_stack = Vec::new();
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
    // Module stack — tracks the active module during build traversal
    // -----------------------------------------------------------------------
    pub fn push_module_comp_init_stack(&mut self, i: ModuleIdent)  { self.module_comp_init_stack.push(i); }
    pub fn pop_module_comp_init_stack (&mut self) -> ModuleIdent   { self.module_comp_init_stack.pop().expect("module stack is empty") }
    pub fn peek_module_comp_init_stack(&self)     -> ModuleIdent   { *self.module_comp_init_stack.last().expect("module stack is empty") }

    pub fn set_module_flow_init_track  (&mut self, i: ModuleIdent) { self.module_flow_init_track = Some(i); }
    pub fn clear_module_flow_init_track(&mut self)                 { self.module_flow_init_track = None; }
    pub fn get_module_flow_init_track  (&self) -> Option<ModuleIdent> { self.module_flow_init_track }
}
