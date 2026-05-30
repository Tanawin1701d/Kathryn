use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug, PartialEq, Eq, Default)]
pub struct NodeTrigger {
    pub hold_node_i      : Option<NcpIdent>,
    pub int_reset_node_i : Option<NcpIdent>,
    pub int_start_node_i : Option<NcpIdent>,
    pub mrst_node_i      : Option<NcpIdent>,
    pub clk_node_i       : Option<NcpIdent>, // clock source node for this trigger group
        depend_nodes     : Vec<(NcpIdent, Option<HcpIdent>)>,
}

impl NodeTrigger {
    pub fn new() -> Self {
        Self {
            hold_node_i      : None,
            int_reset_node_i : None,
            int_start_node_i : None,
            mrst_node_i      : None,
            clk_node_i       : None,
            depend_nodes     : Vec::new(),
        }
    }

    pub fn push_depend_node(&mut self, srci: NcpIdent, condi: Option<HcpIdent>) {
        self.depend_nodes.push((srci, condi));
    }

    pub fn iter_depend_nodes<'a>(&'a self) -> impl Iterator<Item = (NcpIdent, Option<HcpIdent>)> + 'a {
        self.depend_nodes.iter().copied()
    }

    pub fn depend_count(&self) -> usize { self.depend_nodes.len() }

    /// Copy control fields from a flow-block's NodeTrigger into this node's
    /// NodeTrigger.  Pass `with_int_start: true` for nodes that propagate it.
    pub fn fill_ext_node(&mut self, src: &NodeTrigger, with_int_start: bool) {
        self.hold_node_i      = src.hold_node_i;
        self.int_reset_node_i = src.int_reset_node_i;
        self.mrst_node_i      = src.mrst_node_i;
        self.clk_node_i       = src.clk_node_i;
        if with_int_start {
            self.int_start_node_i = src.int_start_node_i;
        }
    }

    pub fn is_unpred_cycle_usage(&self) -> bool {
        self.hold_node_i     .is_some() ||
        self.int_reset_node_i.is_some() ||
        self.int_start_node_i.is_some()
    }
}

pub trait HasNodeTriggerSig {
    fn get_node_triggers    (&self)     -> &NodeTrigger;
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger;

    fn get_hold_node     (&self) -> Option<NcpIdent> { self.get_node_triggers().hold_node_i }
    fn get_int_reset_node(&self) -> Option<NcpIdent> { self.get_node_triggers().int_reset_node_i }
    fn get_int_start_node(&self) -> Option<NcpIdent> { self.get_node_triggers().int_start_node_i }
    fn get_mrst_node     (&self) -> Option<NcpIdent> { self.get_node_triggers().mrst_node_i }
    fn get_clk_node      (&self) -> Option<NcpIdent> { self.get_node_triggers().clk_node_i }

    fn set_hold_node     (&mut self, n: NcpIdent) { self.get_node_triggers_mut().hold_node_i      = Some(n); }
    fn set_int_reset_node(&mut self, n: NcpIdent) { self.get_node_triggers_mut().int_reset_node_i = Some(n); }
    fn set_int_start_node(&mut self, n: NcpIdent) { self.get_node_triggers_mut().int_start_node_i = Some(n); }
    fn set_mrst_node     (&mut self, n: NcpIdent) { self.get_node_triggers_mut().mrst_node_i      = Some(n); }
    fn set_clk_node      (&mut self, n: NcpIdent) { self.get_node_triggers_mut().clk_node_i       = Some(n); }

    fn add_depend_node(&mut self, srci: NcpIdent, condi: Option<HcpIdent>) {
        self.get_node_triggers_mut().push_depend_node(srci, condi);
    }

    /// Copy hold / int-reset / mrst / clk (and optionally int-start) from the
    /// flow-block's NodeTrigger onto this node's NodeTrigger.
    fn fill_ext_node(&mut self, src: &NodeTrigger, with_int_start: bool) {
        self.get_node_triggers_mut().fill_ext_node(src, with_int_start);
    }
}
