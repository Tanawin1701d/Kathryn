pub const DEFAULT_UE_PRI_USER          : i32 = 10;
pub const DEFAULT_UE_PRI_INTERNAL_MAX  : i32 = 100;
pub const DEFAULT_UE_PRI_INTERNAL_MIN  : i32 = 50;
pub const DEFAULT_UE_PRI_RST           : i32 = i32::MAX;
pub const DEFAULT_UE_PRI_MIN           : i32 = 0;
pub const DEFAULT_UE_SUB_PRIORITY_USER : u64 = 0;

use std::rc::Rc;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::operable::Operable;
use crate::model::hw_component::common::slice::Slice;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum UeType {
    Basic  = 0,
    Grp    = 1,
    Cond   = 2,
    Switch = 3,
    Untype = 4,
}

struct UeCommon {
    ue_type      : UeType,
    priority     : i32,
    sub_priority : u64,
    clk_mode     : ClockMode,
}
pub trait UpdateEvent {

    /// ////////////////
    /// common function
    /// ////////////////

    /// retrival function
    fn get_type    (&self)     -> UeType { self.get_ue_common().ue_type      }
    fn get_priority(&self)     -> i32    { self.get_ue_common().priority     }
    fn get_sub_priority(&self) -> u64    { self.get_ue_common().sub_priority }
    fn get_clk_mode(&self) -> ClockMode  { self.get_ue_common().clk_mode     }
    fn is_joinable(&self, rhs: &dyn UpdateEvent) -> bool{

        return self.get_priority() == rhs.get_priority() &&
               self.get_clk_mode() == rhs.get_clk_mode()
    }
    /// assign function
    fn set_priority(&mut self, priority: i32) {
        self.get_ue_common_mut().priority = priority;
    }
    fn set_sub_priority(&mut self, sub_priority: u64) {
        self.get_ue_common_mut().sub_priority = sub_priority;
    }
    fn set_clk_mode(&mut self, clk_mode: ClockMode) {
        self.get_ue_common_mut().clk_mode = clk_mode;
    }

    /// ////////////////
    /// virtual function
    /// ////////////////
    fn get_ue_common    (&self) -> &UeCommon;
    fn get_ue_common_mut(&mut self) -> &mut UeCommon;
    fn get_dep          (&self, result_dep: &mut Vec<Rc<dyn Operable>>);
    fn is_leaf          (&self) -> bool;
}

impl Default for UeCommon {
    fn default() -> Self {
        Self {
            ue_type      : UeType::Untype,
            priority     : DEFAULT_UE_PRI_USER,
            sub_priority : DEFAULT_UE_SUB_PRIORITY_USER,
            clk_mode     : ClockMode::ClkUnused,
        }
    }
}

/**
    UPDATE EVENT BASIC
*/
struct UeBasic{
    ue_common : UeCommon,
    src_val   : Rc<dyn Operable>,
    des_slice : Slice

}

impl UpdateEvent for UeBasic{
    fn get_ue_common(&self) -> &UeCommon{&self.ue_common}
    fn get_ue_common_mut(&mut self) -> &mut UeCommon{&mut self.ue_common}
    fn get_dep(&self, result_dep: &mut Vec<Rc<dyn Operable>>) {
        result_dep.push(self.src_val.clone());
    }
    fn is_leaf(&self) -> bool{true}
}


struct UeGrp{
    ue_common : UeCommon,
}



