use crate::model::hw_component::common::hcp_assign::HcpAssign;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::memEle::MemEle;

pub struct MemBlk {

    assign            : HcpAssign,
    ident             : HcpIdent,
    bit_width         : i32,
    index_width       : i32,
    mem_ele_track_vec : Vec<HcpIdent>,

}

impl MemBlk {



    pub fn new(is_user_com: bool, name: &str, bit_width: i32, index_width: i32) -> HcpIdent {

        assert!(bit_width   > 0, "bit_width must be positive, got {}"  , bit_width  );
        assert!(index_width > 0, "index_width must be positive, got {}", index_width);
        
        let r = MemBlk {
            assign : HcpAssign::new(),
            ident  : HcpIdent::new(HwComponentType::MemBlock,
                                  is_user_com,
                                  name
            ),
            bit_width,
            index_width,
            mem_ele_track_vec: Vec::new()
        };

        r.clone_ident()


    }


    pub fn mk(name: &str, bit_width: i32, index_width: i32) -> HcpIdent {
        MemBlk::new(true, name, bit_width, index_width)
    }

    pub fn clone_ident(&self) -> HcpIdent { self.ident.clone() }

    pub fn create_op(&mut self,
                     is_user_com: bool,
                     name       : &str,
                     index_ident: HcpIdent,
                     is_read    : bool) -> HcpIdent {
        // TODO add to arena
        let mem_ele = MemEle::new(is_user_com,
                                          name,
                                          index_ident,
                                          self.bit_width,
                                          is_read);
        self.mem_ele_track_vec.push(mem_ele.clone_hcp_rdb_ident());
        
        mem_ele.clone_hcp_rdb_ident()
    }
}

impl Identifiable for MemBlk {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}


