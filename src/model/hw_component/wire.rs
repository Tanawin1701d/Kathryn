use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::controller::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MIN, DEFAULT_UE_PRI_MIN};
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// Combinational wire; always ClkFree, no state.
#[derive(Default)]
pub struct Wire {
    assign   : HcpAssign,  // update-event pool and assignable dispatch
    ident    : HcpIdent,   // arena handle + HW type tag
    bit_width: i32,        // signal width in bits

    // User-supplied fallback value driven combinationally at internal-low priority,
    // so any real assignment (higher priority) overrides it (None = no default).
    default_val: Option<HcpIdent>,

    // When false, no fallback event is built at all (not even the implicit zero) —
    // used for externally driven wires like clk / master-reset IO inputs.
    build_default: bool,
}

impl Wire {
    // ---- constructors ----

    /// Full constructor; `is_user_com` false for system-generated wires.
    pub fn new(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign       : HcpAssign::new(),
            ident        : HcpIdent::new(HwComponentType::Wire, HcpSensitiveType::Combinational, is_user_com, name),
            bit_width,
            default_val  : None,
            build_default: true,   // every wire gets a fallback unless explicitly disabled
        }
    }

    // ---- accessors ----

    pub fn get_ident    (&    self) ->      HcpIdent { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }

    // ---- default event ----

    /// Record the fallback value; the event itself is built later in `try_build_default`.
    pub fn set_default_val(&mut self, default_val: HcpIdent) {
        // the caller is responsed to give the match value, we cannot check because arena is not used here
        if self.default_val.is_some() { panic!("default_val is set already") }
        if !self.build_default { panic!("cannot set default_val on a no-default wire") }
        self.default_val = Some(default_val);
    }

    /// Opt this wire out of any fallback event (not even the implicit zero); for
    /// externally driven wires such as clk / master-reset IO inputs.
    pub fn disable_default(&mut self) {
        if self.default_val.is_some() { panic!("cannot disable default on a wire with a default_val") }
        self.build_default = false;
    }

    /// Bind the wire's fallback (combinational, ClkFree). When the user set an explicit
    /// default it wins at internal-low priority; otherwise an implicit zero is bound at
    /// the absolute-min priority so an undriven wire reads 0. Either way the fallback
    /// loses to every real assignment.
    pub fn try_build_default(&mut self, arena: &mut ModelArena) {
        if !self.build_default { return }
        if let Some(default_val_i) = self.default_val {
            // explicit user default — beats the implicit zero, loses to real assignments
            let src_sl = arena.get_hw_slice(&default_val_i);
            self.bind_src(
                default_val_i,
                None, src_sl,
                Some(DEFAULT_UE_PRI_INTERNAL_MIN),
                None, None,
                arena);
        } else {
            // implicit zero default — last resort, lowest possible priority
            let zero_val_i = arena.make_val(
                false,
                &format!("{}_DEFAULT_ZERO", self.ident.get_ident_base().get_abs_name()),
                self.bit_width,
                0,
            );
            let zero_sl = arena.get_hw_slice(&zero_val_i);
            self.bind_src(
                zero_val_i,
                None, zero_sl,
                Some(DEFAULT_UE_PRI_MIN),
                None, None,
                arena);
        }
    }
}



impl HcpAssignable for Wire {
    // ---- pool accessors ----

    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    // ---- Wire-specific overrides ----

    // Wires are always combinational; the Verilog backend relies on ClkFree to
    // emit a continuous assign rather than a clocked always-block.
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    // Full-width destination slice — wires have no sub-field partial writes.
    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    // Delegates to the shared helper; the destination is self (read via get_ident).
    fn do_asm(&self,
              srci     : HcpIdent,
              des_slice: Option<Slice>,
              src_slice: Slice,
              arena    : &mut ModelArena) -> NcpIdent {
        self.gen_asm_node(srci, des_slice, src_slice, arena)
    }
}


// All three delegate to the inner HcpIdent, which forwards to its IdentBase.
impl Identifiable for Wire {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for Wire {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Wire {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_wire(*self); }
}
