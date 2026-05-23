use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::common::update_event_vb::VerilogUpdateEvent;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::update_event::UeType;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

// ---- HcpBaseVb arena extension ----

impl ModelArena {

    /// Take any HCP as a `HcpBaseVb` trait object.
    /// ONE match lives here — callers stay match-free.
    /// When a new HwComponentType is added, add one arm here and
    /// `impl HcpBaseVb` for the new type; nothing else changes.
    pub fn take_hcp_vb(&mut self, ident: HcpIdent) -> Box<dyn HcpBaseVb> {
        match ident.get_hw_type() {
            HwComponentType::Reg               => Box::new(self.take_reg             (ident)),
            HwComponentType::Wire              => Box::new(self.take_wire            (ident)),
            HwComponentType::IoWire            => Box::new(self.take_io_wire         (ident)),
            HwComponentType::Val               => Box::new(self.take_val             (ident)),
            HwComponentType::MemBlock          => Box::new(self.take_mem_blk         (ident)),
            HwComponentType::MemBlockIndexer   => Box::new(self.take_mem_ele         (ident)),
            HwComponentType::Expression        => Box::new(self.take_expression      (ident)),
            HwComponentType::StateReg          => Box::new(self.take_state_reg       (ident)),
            HwComponentType::SyncReg           => Box::new(self.take_sync_reg        (ident)),
            HwComponentType::CntReg            => Box::new(self.take_cnt_reg         (ident)),
            HwComponentType::CondWaitStateReg  => Box::new(self.take_cond_wait_reg   (ident)),
            HwComponentType::CycleWaitStateReg => Box::new(self.take_cycle_wait_reg  (ident)),
            HwComponentType::Nest              => panic!("take_hcp_vb: Nest not yet ported to Verilog backend"),
        }
    }

    /// Put the HCP back — zero match, each type knows its own arena slot.
    pub fn replace_back_hcp_vb(&mut self, v: Box<dyn HcpBaseVb>) { v.replace_back_into_arena_vb(self); }
}

// ---- VerilogUpdateEvent arena extension ----

impl ModelArena {

    /// Take an update event as a `VerilogUpdateEvent` trait object.
    /// ONE match lives here — `transpile_ue` stays match-free.
    /// When a new UeType is added, add one arm here and `impl VerilogUpdateEvent`
    /// for the new type; nothing else changes.
    pub fn take_ue_vb(&mut self, ident: UpdateEventIdent) -> Box<dyn VerilogUpdateEvent> {
        match ident.get_ue_type() {
            UeType::Basic  => Box::new(self.take_ue_basic (ident)),
            UeType::Grp    => Box::new(self.take_ue_grp   (ident)),
            UeType::Cond   => Box::new(self.take_ue_cond  (ident)),
            UeType::Switch => Box::new(self.take_ue_switch (ident)),
            UeType::Untype => panic!("take_ue_vb: UeType::Untype"),
        }
    }
}
