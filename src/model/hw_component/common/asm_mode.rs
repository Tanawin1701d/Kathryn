use std::cell::Cell;
use crate::model::hw_component::common::update_event::DEFAULT_UE_PRI_USER;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum AsmNodePriorityMode {
    Auto,
    Manual,
}

thread_local! {
    static CUR_MODE    : Cell<AsmNodePriorityMode> = Cell::new(AsmNodePriorityMode::Auto);
    static CUR_PRIORITY: Cell<i32>                 = Cell::new(DEFAULT_UE_PRI_USER);
}

pub fn get_asm_pri_mode() -> AsmNodePriorityMode {
    CUR_MODE.get()
}

pub fn get_asm_pri_val() -> i32 {
    CUR_PRIORITY.get()
}

pub fn set_asm_pri_to_auto() {
    CUR_MODE    .set(AsmNodePriorityMode::Auto);
    CUR_PRIORITY.set(DEFAULT_UE_PRI_USER);
}

pub fn set_asm_pri_to_manual(priority: i32) {
    CUR_MODE    .set(AsmNodePriorityMode::Manual);
    CUR_PRIORITY.set(priority);
}
