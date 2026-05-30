use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::util::check_ident_bit_size;
use crate::model::model_arena::ModelArena;

#[derive(Clone, Debug, PartialEq, Eq, Default)]
pub struct TriggerSig {
    pub hold_sig_i      : Option<HcpIdent>,
    pub int_rst_sig_i   : Option<HcpIdent>,
    pub int_start_sig_i : Option<HcpIdent>,
    pub mrst_sig_i      : Option<HcpIdent>,
    pub clk_sig_i       : Option<HcpIdent>, // clock source driving this sp_reg

    depend_sig_i        : Vec<(HcpIdent, Option<HcpIdent>)>,
}

impl TriggerSig {
    pub fn new() -> Self {
        Self {
            hold_sig_i      : None,
            int_rst_sig_i   : None,
            int_start_sig_i : None,
            mrst_sig_i      : None,
            clk_sig_i       : None,
            depend_sig_i    : Vec::new(),
        }
    }

    pub fn push_depend_node(&mut self, srci: HcpIdent, condi: Option<HcpIdent>) {
        self.depend_sig_i.push((srci, condi));
    }

    pub fn iter_depend_nodes(&self) -> impl Iterator<Item = (HcpIdent, Option<HcpIdent>)> + '_ {
        self.depend_sig_i.iter().copied()
    }

    pub fn integrity_check(&self, owner_name: &str, arena: &ModelArena) {
        for sig in [self.hold_sig_i, self.int_rst_sig_i, self.int_start_sig_i, self.mrst_sig_i, self.clk_sig_i].iter().flatten() {
            check_ident_bit_size(sig, 1, owner_name, arena);
        }
        for (src, cond) in self.iter_depend_nodes() {
            check_ident_bit_size(&src, 1, owner_name, arena);
            if let Some(c) = cond {
                check_ident_bit_size(&c, 1, owner_name, arena);
            }
        }
    }

}

pub trait HasTriggerSig {
    fn get_triggers    (&self)     -> &TriggerSig;
    fn get_triggers_mut(&mut self) -> &mut TriggerSig;

    /// Replace this sp_reg's TriggerSig wholesale.  Used by node `assign`
    /// flows that build a complete TriggerSig from the node's NodeTrigger.
    fn set_triggers(&mut self, sig: TriggerSig) { *self.get_triggers_mut() = sig; }

    fn get_hold_sig_i(&self) -> Option<HcpIdent> { self.get_triggers().hold_sig_i }
    fn get_rst_sig_i (&self) -> Option<HcpIdent> { self.get_triggers().int_rst_sig_i }
    fn get_int_sig_i (&self) -> Option<HcpIdent> { self.get_triggers().int_start_sig_i }
    fn get_mrst_sig_i(&self) -> Option<HcpIdent> { self.get_triggers().mrst_sig_i }
    fn get_clk_sig_i (&self) -> Option<HcpIdent> { self.get_triggers().clk_sig_i }

    fn set_hold_sig_i(&mut self, ident: HcpIdent) { self.get_triggers_mut().hold_sig_i       = Some(ident); }
    fn set_rst_sig_i (&mut self, ident: HcpIdent) { self.get_triggers_mut().int_rst_sig_i    = Some(ident); }
    fn set_int_sig_i (&mut self, ident: HcpIdent) { self.get_triggers_mut().int_start_sig_i  = Some(ident); }
    fn set_clk_sig_i (&mut self, ident: HcpIdent) { self.get_triggers_mut().clk_sig_i        = Some(ident); }

    fn add_depend_node(&mut self, srci: HcpIdent, condi: Option<HcpIdent>) {
        self.get_triggers_mut().push_depend_node(srci, condi);
    }

    fn check_all_sigs_1bit(&self, owner_name: &str, arena: &ModelArena) {
        self.get_triggers().integrity_check(owner_name, arena);
    }
}
