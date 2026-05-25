use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::hw_component::common::slice::Slice;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::memEle::MemEle;
use crate::model::model_arena::ModelArena;

#[derive(Default)]
pub struct MemBlk {

    assign            : HcpAssign,
    ident             : HcpIdent,
    bit_width         : i32,
    index_width       : i32,
    mem_ele_track_vec : Vec<HcpIdent>,

}

impl MemBlk {



    pub fn new(is_user_com: bool, name: &str, bit_width: i32, index_width: i32) -> Self {
        assert!(bit_width   > 0, "bit_width must be positive, got {}"  , bit_width  );
        assert!(index_width > 0, "index_width must be positive, got {}", index_width);
        Self {
            assign           : HcpAssign::new(),
            ident            : HcpIdent::new(HwComponentType::MemBlock, is_user_com, name),
            bit_width,
            index_width,
            mem_ele_track_vec: Vec::new(),
        }
    }

    pub fn mk(name: &str, bit_width: i32, index_width: i32) -> Self {
        MemBlk::new(true, name, bit_width, index_width)
    }

    pub fn get_ident      (&self)     -> HcpIdent      { self.ident }
    pub fn get_ident_mut  (&mut self) -> &mut HcpIdent { &mut self.ident }
    pub fn get_bit_width  (&self)     -> i32            { self.bit_width   }
    pub fn get_index_width(&self)     -> i32            { self.index_width }

    pub fn create_op(&mut self,
                     is_user_com: bool,
                     name       : &str,
                     index_ident: HcpIdent,
                     is_read    : bool) -> HcpIdent {
        // TODO add to arena
        let mem_ele = MemEle::new(is_user_com, name, self.ident, index_ident, self.bit_width, is_read);
        let ident   = mem_ele.get_ident();
        self.mem_ele_track_vec.push(ident);
        ident
    }
}

impl Identifiable for MemBlk {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpAssignable for MemBlk {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { panic!("MemBlk is not HcpAssignable") }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { panic!("MemBlk is not HcpAssignable") }
    fn retrieve_clk_mode (&self)     -> ClockMode       { panic!("MemBlk is not HcpAssignable") }
    fn get_des_slice     (&self)     -> Slice            { panic!("MemBlk is not HcpAssignable") }
    fn get_priority      (&self)     -> i32              { panic!("MemBlk is not HcpAssignable") }
    fn do_asm(&self, _: &HcpIdent, _: &Option<Slice>, _: &Slice, _: &mut ModelArena) -> AssignMeta {
        panic!("MemBlk is not HcpAssignable")
    }
}

impl HcpIdentifiable for MemBlk {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for MemBlk {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_blk(*self); }
}
