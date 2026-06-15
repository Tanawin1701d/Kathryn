use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::controller::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// Memory element: a single indexed access (read or write) into a MemBlk.
// Acts as a proxy signal — its name in Verilog is derived from the parent MemBlk's name.
#[derive(Default)]
pub struct MemEle {
    assign           : HcpAssign,
    ident            : HcpIdent,
    master_mem_blk_i : HcpIdent, // owning MemBlk (name needed for Verilog emission)
    index_ident      : HcpIdent, // the address/index signal driving this access
    bit_width        : i32,      // must match the owning MemBlk's bit_width
    is_read          : bool,     // true = read port, false = write port
}

impl MemEle {
    // ---- constructors ----

    /// Full constructor; `master_mem_blk_i` links this element back to its owning block.
    pub fn new(is_user_com      : bool,
               name             : &str,
               master_mem_blk_i : HcpIdent,
               index_ident      : HcpIdent,
               bit_width        : i32,
               is_read          : bool,
    ) -> Self {
        assert!(bit_width > 0, "bit_width must be positive, got {}", bit_width);
        Self {
            assign          : HcpAssign::new(),
            ident           : HcpIdent::new(HwComponentType::MemBlockIndexer, HcpSensitiveType::Clocked, is_user_com, name),
            master_mem_blk_i,
            index_ident,
            bit_width,
            is_read,
        }
    }

    // ---- accessors ----

    pub fn get_ident            (&    self) ->      HcpIdent { self.ident            }
    pub fn get_ident_mut        (&mut self) -> &mut HcpIdent { &mut self.ident       }
    pub fn get_master_mem_blk_i (&    self) ->      HcpIdent { self.master_mem_blk_i }
    pub fn get_index_ident      (&    self) ->      HcpIdent { self.index_ident      }
    pub fn is_read              (&    self) ->      bool     { self.is_read          }
}

impl HcpAssignable for MemEle {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { get_global_clk_mode() }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci: HcpIdent,
              des_slice: Option<Slice>,
              src_slice: Slice,
              arena    : &mut ModelArena) -> NcpIdent {
        self.gen_asm_node(srci, des_slice, src_slice, arena)
    }
}


impl Identifiable for MemEle {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for MemEle {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for MemEle {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_ele(*self); }
}
