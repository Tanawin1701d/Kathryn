use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::complex_hardware::common::ccp_base::CcpBase;
use crate::model::complex_hardware::common::ccp_hw_build::mux_into_wire;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// ---- DynCounter -------------------------------------------------------------

/// Dynamic accumulate counter — a CCP wrapping ONE clocked register plus a
/// combinational add chain grown statement by statement:
///
///   new(width)          -> `width`-bit register (value wraps mod 2^width)
///   add(k, Some(en))    -> stage wire = en ? prev + k : prev   (2:1 comb mux)
///   add(k, None)        -> stage      = prev + k               (plain adder)
///   update()            -> reg <= chain head, ONE basic node attached to the
///                          current scope (clk wired by the enclosing build)
///
/// `prev` is the previous stage (the register itself for the first add), so
/// simultaneously-enabled adds accumulate in one cycle.  `update` consumes the
/// chain: the next `add` starts again from the register.
pub struct DynCounter {
    ident     : CcpIdent         ,
    width     : i32              ,
    cnt_reg_i : HcpIdent         ,    // the committed value (clocked)
    pending_i : Option<HcpIdent> ,    // head of the uncommitted add chain
    stage_cnt : u32              ,    // stages created so far — naming only
}

impl Default for DynCounter {
    fn default() -> Self {
        Self {
            ident     : CcpIdent::new(CcpType::DynCounter, false, ""),
            width     : 0,
            cnt_reg_i : HcpIdent::default(),
            pending_i : None,
            stage_cnt : 0,
        }
    }
}

impl DynCounter {
    pub fn new(is_user_com: bool, name: &str, width: i32, arena: &mut ModelArena) -> Self {
        assert!(width >= 1, "DynCounter::new: width must be >= 1, got {width}");
        let cnt_reg_i = arena.make_reg(false, &format!("{name}_CNT"), width);
        Self {
            ident     : CcpIdent::new(CcpType::DynCounter, is_user_com, name),
            width,
            cnt_reg_i,
            pending_i : None,
            stage_cnt : 0,
        }
    }

    // ---- accessors ---------------------------------------------------------
    pub fn get_ccp_ident    (&self)     -> CcpIdent      { self.ident }
    pub fn get_ccp_ident_mut(&mut self) -> &mut CcpIdent { &mut self.ident }
    pub fn get_width        (&self)     -> i32           { self.width }
    pub fn get_reg_i        (&self)     -> HcpIdent      { self.cnt_reg_i }
    pub fn get_now_i        (&self)     -> HcpIdent      { self.pending_i.unwrap_or(self.cnt_reg_i) }

    // ---- operations --------------------------------------------------------

    /// Chain one add stage onto the counter: `enable ? prev + addend : prev`
    /// (plain `prev + addend` without an enable).  Returns the new chain head.
    pub fn add(
        &mut self,
        addend_i: HcpIdent        ,
        enable_i: Option<HcpIdent>,
        arena   : &mut ModelArena ,
    ) -> Result<HcpIdent, String> {
        let base  = self.ident.get_ident_base().get_rel_name().to_string();
        let stage = self.stage_cnt;
        self.stage_cnt += 1;

        let prev_i = self.get_now_i();
        let sum_i  = arena.make_expression(false, &format!("{base}_ADD{stage}"),
                                           LogicOp::ArithPlus, prev_i, addend_i, None, None);
        let head_i = match enable_i {
            Some(en_i) => {
                let en_sz = arena.get_hw_bit_sz(&en_i);
                if en_sz != 1 {
                    return Err(format!("DynCounter add: enable must be 1-bit, got {en_sz}"));
                }
                let stage_w_i = arena.make_wire(false, &format!("{base}_ST{stage}"), self.width);
                mux_into_wire(arena, stage_w_i, sum_i, prev_i, en_i);
                stage_w_i
            }
            None => sum_i,
        };
        self.pending_i = Some(head_i);
        Ok(head_i)
    }

    /// Commit the chain head into the register as one clocked basic node in the
    /// current scope; the chain restarts from the register afterwards.
    pub fn update(&mut self, arena: &mut ModelArena) -> Result<(), String> {
        let head_i = self.pending_i.take()
            .ok_or_else(|| "DynCounter update: nothing to commit — call add() first".to_string())?;
        // width drift (an unconditional chain head is an expression that may be
        // wider than the register) is truncated back by the assign sanitizer
        let src_slice = arena.get_hw_slice(&head_i);
        let _resize   = arena.gen_basic_assign(self.cnt_reg_i, head_i, None, src_slice);
        Ok(())
    }
}

impl CcpBase for DynCounter {
    /// All counter hardware is wired eagerly at `add` / `update` time — nothing
    /// left to do at module build.
    fn build(&mut self, _arena: &mut ModelArena) {}
    fn set_ccp_ident(&mut self, ident: CcpIdent) { *self.get_ccp_ident_mut() = ident; }
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_dyn_counter(*self); }
}

impl Identifiable for DynCounter {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}
