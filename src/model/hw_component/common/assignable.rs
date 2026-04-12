

use std::rc::Rc;
use crate::model::hw_component::common::update_event::UpdatingEvent;
use crate::model::hw_component::common::operable::Operable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::controller::clock_mode::ClockMode;

pub trait Assignable {

    /// ////////////////
    /// virtual function
    /// ////////////////


    fn do_clk_edge_asm(&mut self, src_opr  : Rc<dyn Operable>,
                                  des_slice: Slice,
                                  clk_mode : ClockMode);

    fn do_level_asm(&mut self, src_opr  : Rc<dyn Operable>,
                               des_slice: Slice
    );
}