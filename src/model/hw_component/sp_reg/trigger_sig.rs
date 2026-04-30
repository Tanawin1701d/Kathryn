use crate::model::hw_component::common::hcp_ident::HcpIdent;

pub const MAX_DEPEND_NODES: usize = 16;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct TriggerSig {
    pub hold_sig_i  : Option<HcpIdent>,
    pub rst_sig_i   : Option<HcpIdent>,
    pub int_sig_i   : Option<HcpIdent>,
    pub mrst_sig_i  : Option<HcpIdent>,
    depend_nodes    : [Option<(HcpIdent, Option<HcpIdent>)>; MAX_DEPEND_NODES],
    depend_count    : usize,
}

impl TriggerSig {
    pub fn new() -> Self {
        Self {
            hold_sig_i   : None,
            rst_sig_i    : None,
            int_sig_i    : None,
            mrst_sig_i   : None,
            depend_nodes : [None; MAX_DEPEND_NODES],
            depend_count : 0,
        }
    }

    pub fn push_depend_node(&mut self, srci: HcpIdent, condi: Option<HcpIdent>) {
        assert!(self.depend_count < MAX_DEPEND_NODES, "depend_nodes capacity exceeded");
        self.depend_nodes[self.depend_count] = Some((srci, condi));
        self.depend_count += 1;
    }

    pub fn iter_depend_nodes(&self) -> impl Iterator<Item = (HcpIdent, Option<HcpIdent>)> + '_ {
        self.depend_nodes[..self.depend_count].iter().map(|n| n.unwrap())
    }
}

pub trait HasTriggerSig {
    fn get_triggers    (&self)     -> &TriggerSig;
    fn get_triggers_mut(&mut self) -> &mut TriggerSig;

    fn get_hold_sig_i(&self) -> Option<HcpIdent> { self.get_triggers().hold_sig_i }
    fn get_rst_sig_i (&self) -> Option<HcpIdent> { self.get_triggers().rst_sig_i  }
    fn get_int_sig_i (&self) -> Option<HcpIdent> { self.get_triggers().int_sig_i  }
    fn get_mrst_sig_i(&self) -> Option<HcpIdent> { self.get_triggers().mrst_sig_i }

    fn set_hold_sig_i(&mut self, ident: HcpIdent) { self.get_triggers_mut().hold_sig_i = Some(ident); }
    fn set_rst_sig_i (&mut self, ident: HcpIdent) { self.get_triggers_mut().rst_sig_i  = Some(ident); }
    fn set_int_sig_i (&mut self, ident: HcpIdent) { self.get_triggers_mut().int_sig_i  = Some(ident); }

    fn add_depend_node(&mut self, srci: HcpIdent, condi: Option<HcpIdent>) {
        self.get_triggers_mut().push_depend_node(srci, condi);
    }
}
