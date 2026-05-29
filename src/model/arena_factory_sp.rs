use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::hw_component::sp_reg::sync_reg::SyncReg;
use crate::model::hw_component::sp_reg::cnt_reg::CntReg;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::sp_reg::wait_reg::{CondWaitStateReg, CycleWaitStateReg};
use crate::model::model_arena::ModelArena;

impl ModelArena {
    pub fn make_state_reg(&mut self, name: &str) -> HcpIdent {
        let sr = StateReg::new(false, name, self);
        let i  = self.add_state_reg(sr);
        self.stamp_hw_to_parent_module(i, false)
    }

    pub fn make_sync_reg(&mut self, name: &str, size: i32) -> HcpIdent {
        let i = self.add_sync_reg(SyncReg::new(false, name, size));
        self.stamp_hw_to_parent_module(i, false)
    }

    pub fn make_cnt_reg(&mut self, name: &str, inc_val: i32, last_cycle: i32) -> HcpIdent {
        let i = self.add_cnt_reg(CntReg::new(false, name, inc_val, last_cycle));
        self.stamp_hw_to_parent_module(i, false)
    }

    pub fn make_cond_wait_state_reg(&mut self, name: &str, cond_opr: HcpIdent, cond_sl: Slice) -> HcpIdent {
        let i = self.add_cond_wait_reg(CondWaitStateReg::new(false, name, cond_opr, cond_sl));
        self.stamp_hw_to_parent_module(i, false)
    }

    pub fn make_cycle_wait_state_reg(&mut self, name: &str, wait_cycle: i32) -> HcpIdent {
        let end_cnt_i = self.make_val(false, &format!("{}_END_CNT", name), 1, wait_cycle as u64);
        let i = self.add_cycle_wait_reg(CycleWaitStateReg::new_with_cycle(false, name, wait_cycle, end_cnt_i));
        self.stamp_hw_to_parent_module(i, false)
    }

    pub fn make_cycle_wait_state_reg_with_expr(&mut self, name: &str, cnt_bit_sz: i32, end_cnt_i: HcpIdent) -> HcpIdent {
        let i = self.add_cycle_wait_reg(CycleWaitStateReg::new_with_expr(false, name, cnt_bit_sz, end_cnt_i));
        self.stamp_hw_to_parent_module(i, false)
    }
}
