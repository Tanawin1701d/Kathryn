use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::flow_block::common::PipSchematic;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// Pipeline flow block: a single body sub-block gated by an arbiter (Arb CCP).
// The body runs while the arb grants a master request and stalls (wait4syn)
// while it does not.  All node wiring lives in PipSchematic.
#[derive(Clone, Debug)]
pub struct FlowBlockPip {
    base        : FlowBlockBase,
    schematic   : PipSchematic,
    result      : Option<NodeWrap>,
    // auto-restart: route the arb user-reset into the block's start signal so a
    // reset re-launches the pipeline instead of just clearing it.
    auto_restart: bool,
}

impl Default for FlowBlockPip {
    fn default() -> Self { Self::new("", CcpIdent::new(CcpType::Arb, false, "")) }
}

impl FlowBlockPip {
    pub fn new             (name: &str, arb_i: CcpIdent) -> Self { Self::make(name, arb_i, false) }
    pub fn new_auto_restart(name: &str, arb_i: CcpIdent) -> Self { Self::make(name, arb_i, true)  }

    fn make(name: &str, arb_i: CcpIdent, auto_restart: bool) -> Self {
        Self {
            base        : FlowBlockBase::new(FlowBlockType::Pipeline, FlowBlockJoinPolicy::SubFlow, name, false),
            schematic   : PipSchematic::new(arb_i),
            result      : None,
            auto_restart,
        }
    }

    // Route the backing arb's optional user hold / reset into this block's
    // ext_signals before the standard build wires the trigger node.
    fn feed_arb_ext_signals(&mut self, arena: &mut ModelArena) {
        // pull the optional user hold / reset off the backing arb.
        let (user_hold_i, user_reset_i) = {
            let arb          = arena.take_arb(self.schematic.get_arb_i());
            let user_hold_i  = arb.get_user_hold_i();
            let user_reset_i = arb.get_user_reset_i();
            arena.replace_back_arb(arb);
            (user_hold_i, user_reset_i)
        };

        // feed the arb's hold / reset into this block's ext_signals so the
        // trigger node joins them like any other block-level hold / reset.
        if let Some(hold_i)  = user_hold_i  { self.base.add_int_signal(ExtSigType::Hold,  hold_i);  }
        if let Some(reset_i) = user_reset_i { self.base.add_int_signal(ExtSigType::Reset, reset_i); }

        // auto-restart: the arb reset also re-launches the pipeline, so route it
        // into the start signal as well.
        if self.auto_restart {
            if let Some(reset_i) = user_reset_i { self.base.add_int_signal(ExtSigType::Start, reset_i); }
        }
    }
}

impl FlowBlock for FlowBlockPip {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("pipeline block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert!(self.base.get_sub_blocks_i().is_empty(), "pipeline block already has a body sub-block");
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_pip(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn check_prefinalize(&self) -> Result<(), String> {
        if self.base.get_sub_blocks_i().len() != 1 {
            return Err("pipeline block must have exactly one body sub-block".to_string());
        }
        if !self.base.get_con_blocks_i().is_empty() {
            return Err("pipeline block does not support con blocks".to_string());
        }
        Ok(())
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("pipeline block has not been built")
    }

    fn build_hw_master(&mut self, arena: &mut ModelArena) {
        self.feed_arb_ext_signals(arena);
        self.build_hw_master_base(arena);
    }
}

impl Identifiable for FlowBlockPip {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
