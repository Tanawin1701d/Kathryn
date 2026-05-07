use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::hw_component::sp_reg::sync_reg::SyncReg;
use crate::model::hw_component::sp_reg::cnt_reg::CntReg;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::sp_reg::wait_reg::{CondWaitStateReg, CycleWaitStateReg};
use crate::model::model_arena::ModelArena;

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    pub fn make_state_reg(&mut self, name: &str) -> HcpIdent {
        let set_val_i   = self.make_val(&format!("{}_SET",   name), 1, 1);
        let unset_val_i = self.make_val(&format!("{}_UNSET", name), 1, 0);
        self.add_state_reg(StateReg::new(false, name, set_val_i, unset_val_i))
    }

    pub fn make_sync_reg(&mut self, name: &str, size: i32) -> HcpIdent {
        self.add_sync_reg(SyncReg::new(false, name, size))
    }

    pub fn make_cnt_reg(&mut self, name: &str, inc_val: i32, last_cycle: i32) -> HcpIdent {
        self.add_cnt_reg(CntReg::new(false, name, inc_val, last_cycle))
    }

    pub fn make_cond_wait_state_reg(&mut self, name: &str, cond_opr: HcpIdent, cond_sl: Slice) -> HcpIdent {
        self.add_cond_wait_reg(CondWaitStateReg::new(false, name, cond_opr, cond_sl))
    }

    pub fn make_cycle_wait_state_reg(&mut self, name: &str, wait_cycle: i32) -> HcpIdent {
        let end_cnt_i = self.make_val(&format!("{}_END_CNT", name), 1, wait_cycle as u64);
        self.add_cycle_wait_reg(CycleWaitStateReg::new_with_cycle(false, name, wait_cycle, end_cnt_i))
    }

    pub fn make_cycle_wait_state_reg_with_expr(&mut self, name: &str, cnt_bit_sz: i32, end_cnt_i: HcpIdent) -> HcpIdent {
        self.add_cycle_wait_reg(CycleWaitStateReg::new_with_expr(false, name, cnt_bit_sz, end_cnt_i))
    }
}
