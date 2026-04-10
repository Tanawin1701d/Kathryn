pub const DEFAULT_UE_PRI_USER          : i32 = 10;
pub const DEFAULT_UE_PRI_INTERNAL_MAX  : i32 = 100;
pub const DEFAULT_UE_PRI_INTERNAL_MIN  : i32 = 50;
pub const DEFAULT_UE_PRI_RST           : i32 = i32::MAX;
pub const DEFAULT_UE_PRI_MIN           : i32 = 0;
pub const DEFAULT_UE_SUB_PRIORITY_USER : u64 = 0;

use crate::model::hwComponent::common::operable::CLOCK_MODE;
use crate::model::hwComponent::common::operable::Operable;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum UeType {
    Basic  = 0,
    Grp    = 1,
    Cond   = 2,
    Switch = 3,
}

pub trait UpdateEvent {

    /// retrival function
    fn get_type        (&self) -> UeType;
    fn get_priority    (&self) -> i32;
    fn get_sub_priority(&self) -> u64;
    fn get_clk_mode    (&self) -> CLOCK_MODE;
    fn get_dep         (&self, result_dep: &mut Vec<Box<dyn UpdateEvent>>);
    fn is_leaf         (&self) -> bool;
    fn is_joinable     (&self, rhs: &dyn UpdateEvent) -> bool;

    /// assign function
    fn set_priority    (&mut self, priority    : i32);
    fn set_sub_priority(&mut self, sub_priority: u64);
    fn set_clk_mode    (&mut self, clk_mode    : CLOCK_MODE);

}


