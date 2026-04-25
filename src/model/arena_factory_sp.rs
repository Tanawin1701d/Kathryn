use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::model_arena::ModelArena;

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    pub fn make_state_reg(&mut self, name: &str, set_val_i: HcpIdent, unset_val_i: HcpIdent) -> HcpIdent {
        let s = StateReg::new(false, name, set_val_i, unset_val_i); let i = s.get_ident(); self.add_state_reg(s); i
    }
    pub fn mk_state_reg(&mut self, name: &str, set_val_i: HcpIdent, unset_val_i: HcpIdent) -> HcpIdent {
        let s = StateReg::new(true, name, set_val_i, unset_val_i); let i = s.get_ident(); self.add_state_reg(s); i
    }
}
