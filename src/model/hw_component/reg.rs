use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::controller::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::DEFAULT_UE_PRI_RST;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// General-purpose clocked register; the most common HCP type.
#[derive(Default)]
pub struct Reg {
    assign    : HcpAssign,
    ident     : HcpIdent,
    bit_width : i32,

    reset_val     : Option<HcpIdent>,
    reset_clk_mode: Option<ClockMode>,
}

impl Reg {
    // ---- constructors ----

    /// Full constructor; `is_user_com` false for system-generated regs.
    pub fn new(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign        : HcpAssign::new(),
            ident         : HcpIdent::new(HwComponentType::Reg, HcpSensitiveType::Clocked, is_user_com, name),
            bit_width,
            reset_val     : None,
            reset_clk_mode: None
        }
    }

    /// Shorthand for user-declared regs (`is_user_com = true`).
    pub fn mk(name: &str, bit_width: i32) -> Self {
        Reg::new(true, name, bit_width)
    }

    // ---- accessors ----

    pub fn get_ident(&self) -> HcpIdent { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }

    pub fn set_reset_val(&mut self, reset_val: HcpIdent, reset_clk_mode: ClockMode) {
        // the caller is responsed to give the match value, we cannot check because arena is not used here
        if (self.reset_val.is_some()){ panic!("reset_val is set already")}
        self.reset_val      = Some(reset_val);
        self.reset_clk_mode = Some(reset_clk_mode)
    }

    /// Build the master-reset event: when `mreset_i` fires, load the reset value at the
    /// global RST priority (so it wins over every assignment). Edge-clocked resets draw
    /// the module clk as their source; combinational ones take no clk_src. No-op when the
    /// user never declared a reset value.
    pub fn try_build_reset(&mut self, clk_src: HcpIdent, mreset_i: HcpIdent, arena: &mut ModelArena){
        if self.reset_val.is_none(){return}
        let reset_val_i = self.reset_val.unwrap();
        let src_sl      = arena.get_hw_slice(&reset_val_i);
        let cm          = self.reset_clk_mode.unwrap_or(ClockMode::ClkFree);
        let clk_src_opt = match cm {
            ClockMode::PosEdge | ClockMode::NegEdge => Some(clk_src),
            _                                       => None,
        };
        let ue_i = arena.make_ue_full(
            None                , Some(mreset_i), reset_val_i,
            self.get_des_slice(), src_sl        ,
            DEFAULT_UE_PRI_RST  , cm            , false, clk_src_opt,
        );
        self.add_update_event(ue_i);
    }


}



impl HcpAssignable for Reg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { get_global_clk_mode() }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci       : HcpIdent,
              des_slice  : Option<Slice>,
              src_slice  : Slice,
              arena      : &mut ModelArena,
    ) -> NcpIdent {

        self.gen_asm_node(srci, des_slice, src_slice, arena)
    }
}


impl Identifiable for Reg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for Reg {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Reg {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_reg(*self); }
}
