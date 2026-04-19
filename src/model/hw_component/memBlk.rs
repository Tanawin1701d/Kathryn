use crate::model::hw_component::common::hcp_assign::HcpAssign;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};

pub struct MemEleTrack{
    mem_ele_ident : HcpIdent,
    index_ident   : HcpIdent,
    is_read       : bool,
}

impl MemEleTrack {
    pub fn new(mem_ele_ident : HcpIdent, 
               index_ident   : HcpIdent, 
               is_read       : bool) -> Self {
        MemEleTrack { mem_ele_ident, index_ident, is_read }
    }
}

pub struct MemBlk {

    assign      : HcpAssign,
    ident       : HcpIdent,
    bit_width   : i32,
    index_width : i32,
    mem_ele_track_vec : Vec<MemEleTrack>,

}

impl MemBlk {

    pub fn new(is_user_com: bool, name: &str, bit_width: i32, index_width: i32) -> Self {

        assert!(bit_width   > 0, "bit_width must be positive, got {}"  , bit_width  );
        assert!(index_width > 0, "index_width must be positive, got {}", index_width);
        
        MemBlk {
            assign : HcpAssign::new(),
            ident  : HcpIdent::new(HwComponentType::MemBlock,
                                  is_user_com,
                                  name
            ),
            bit_width,
            index_width,
            mem_ele_track_vec
        }
    }





}