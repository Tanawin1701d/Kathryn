use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::hw_component::common::slice::Slice;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::mem_ele::MemEle;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// Memory block: a 2-D array of `bit_width`-wide words addressed by an `index_width`-bit index.
// MemBlk is a grouper/container — it is never assigned to directly; all assignments flow
// through MemEle indexer instances.  HcpAssignable methods all panic for this reason.
#[derive(Default)]
pub struct MemBlk {

    assign            : HcpAssign,
    ident             : HcpIdent,
    bit_width         : i32,          // data width of each word
    index_width       : i32,          // address width (number of index bits)
    mem_ele_track_vec : Vec<HcpIdent>, // idents of all MemEle ops created from this block

}

impl MemBlk {

    // ---- constructors ----

    /// Full constructor; asserts positive dimensions at creation time.
    pub fn new(is_user_com: bool, name: &str, bit_width: i32, index_width: i32) -> Self {
        assert!(bit_width   > 0, "bit_width must be positive, got {}"  , bit_width  );
        assert!(index_width > 0, "index_width must be positive, got {}", index_width);
        Self {
            assign           : HcpAssign::new(),
            ident            : HcpIdent::new(HwComponentType::MemBlock, HcpSensitiveType::Clocked, is_user_com, name),
            bit_width,
            index_width,
            mem_ele_track_vec: Vec::new(),
        }
    }

    // ---- accessors ----

    pub fn get_ident      (&self)     -> HcpIdent      { self.ident }
    pub fn get_ident_mut  (&mut self) -> &mut HcpIdent { &mut self.ident }
    pub fn get_bit_width  (&self)     -> i32            { self.bit_width   }
    pub fn get_index_width(&self)     -> i32            { self.index_width }

    // ---- MemEle factory ----

    /// Create a read or write accessor (`MemEle`) for this block at the given index signal,
    /// insert it into the arena, and record its ident in `mem_ele_track_vec`.
    pub fn create_op(&mut self,
                     is_user_com: bool,
                     name       : &str,
                     index_ident: HcpIdent,
                     is_read    : bool,
                     arena      : &mut ModelArena) -> HcpIdent {
        let mem_ele = MemEle::new(is_user_com, name, self.ident, index_ident, self.bit_width, is_read);
        let ident   = arena.add_mem_ele(mem_ele);  // ident stamped with arena handle after insert
        self.mem_ele_track_vec.push(ident);
        ident
    }
}

impl Identifiable for MemBlk {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

// MemBlk satisfies HcpAssignable only to fit the trait hierarchy — all methods panic
// because the block itself is never a signal destination.
impl HcpAssignable for MemBlk {
    fn get_hcp_assign    (&    self) -> &    HcpAssign { panic!("MemBlk is not HcpAssignable") }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { panic!("MemBlk is not HcpAssignable") }
    fn retrieve_clk_mode (&    self) -> ClockMode      { panic!("MemBlk is not HcpAssignable") }
    fn get_des_slice     (&    self) -> Slice          { panic!("MemBlk is not HcpAssignable") }
    fn get_priority      (&    self) -> i32            { panic!("MemBlk is not HcpAssignable") }
    fn do_asm(&self, _: HcpIdent, _: Option<Slice>, _: Slice, _: &mut ModelArena) -> NcpIdent {
        panic!("MemBlk is not HcpAssignable")
    }
}

impl HcpIdentifiable for MemBlk {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for MemBlk {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_blk(*self); }
}
