use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::{gen_procedure_blk, signal_width};
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::reg::Reg;
use crate::model::hw_component::sp_reg::cnt_reg::CntReg;
use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::hw_component::sp_reg::sync_reg::SyncReg;
use crate::model::hw_component::sp_reg::wait_reg::{CondWaitStateReg, CycleWaitStateReg};
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

// All register-family types share the same HcpBaseVb shape:
//   - gen_type   : "reg [N-1:0] " from get_des_slice().get_size()
//   - gen_var_name: plain global name
//   - 1 init line (empty), 1 procedure block (clocked)
//   - replace_back varies per concrete type → macro injects the one differing call
macro_rules! impl_reg_vb {
    ($T:ty, $replace_back:ident) => {
        impl HcpBaseVb for $T {
            fn gen_type         (&self) -> String { let w = signal_width(self.get_des_slice().get_size()); format!("reg {w}") }
            fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
            fn amt_init_line    (&self) -> u32    { 1 }
            fn amt_precedure_blk(&self) -> u32    { 1 }

            fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena, _fw: &mut FileWriter) { /* regs need no init declaration */ }
            fn gen_procedure_blk(&self, _idx: u32,  arena: &mut ModelArena,  fw: &mut FileWriter) {
                gen_procedure_blk(self, self.get_ident(), arena, fw)
            }

            fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.$replace_back(*self); }
        }
    };
}

impl_reg_vb!(Reg,               replace_back_reg           );
impl_reg_vb!(StateReg,          replace_back_state_reg     );
impl_reg_vb!(SyncReg,           replace_back_sync_reg      );
impl_reg_vb!(CntReg,            replace_back_cnt_reg       );
impl_reg_vb!(CondWaitStateReg,  replace_back_cond_wait_reg );
impl_reg_vb!(CycleWaitStateReg, replace_back_cycle_wait_reg);
