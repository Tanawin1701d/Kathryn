use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{HasNodeTriggerSig, NcpNode, NodeTrigger};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

pub struct SynNode {
    ident           : NcpIdent,
    clk_mode        : ClockMode,
    triggers        : NodeTrigger,
    sync_reg_i      : HcpIdent,
    syn_size        : i32,
    force_exit_node : Option<NcpIdent>,
    end_expr_i      : Option<HcpIdent>,
    bound_exit_i    : Option<HcpIdent>,
}

impl Default for SynNode {
    fn default() -> Self {
        Self {
            ident          : NcpIdent::new(NodeType::Syn, false, ""),
            clk_mode       : ClockMode::PosEdge,
            triggers       : NodeTrigger::new(),
            sync_reg_i     : HcpIdent::default(),
            syn_size       : 1,
            force_exit_node: None,
            end_expr_i     : None,
            bound_exit_i   : None,
        }
    }
}

impl SynNode {
    pub fn new(is_user_com: bool, name: &str, syn_size: i32, clk_mode: ClockMode, arena: &mut ModelArena) -> Self {
        assert!(syn_size > 0);
        let sync_reg_i = arena.make_sync_reg(&format!("{}_SY", name), syn_size);
        Self {
            ident          : NcpIdent::new(NodeType::Syn, is_user_com, name),
            clk_mode,
            triggers       : NodeTrigger::new(),
            sync_reg_i,
            syn_size,
            force_exit_node: None,
            end_expr_i     : None,
            bound_exit_i   : None,
        }
    }

    pub fn set_force_exit_event(&mut self, nd: NcpIdent) { self.force_exit_node = Some(nd); }
    pub fn get_sync_reg_i(&self) -> HcpIdent { self.sync_reg_i }
    pub fn get_syn_size  (&self) -> i32      { self.syn_size }
}

impl HasNodeTriggerSig for SynNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for SynNode {
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, arena: &mut ModelArena) {
        let force_exit_inv: Option<HcpIdent> = if let Some(fe) = self.force_exit_node {
            let fe_exit = arena.get_node_exit_opr(&fe);
            Some(arena.make_expression(
                &format!("{}_FE_INV", self.ident.get_ident_base().get_name()),
                LogicOp::BitwiseInvr, fe_exit, fe_exit,
                None, None,
            ))
        } else { None };

        let dep_list: Vec<(NcpIdent, Option<HcpIdent>)> =
            self.triggers.iter_depend_nodes().collect();
        let int_rst_sig = self.get_int_reset_node()
            .map(|h| arena.get_node_exit_opr(&h));
        let force_exit_exit = self.force_exit_node
            .map(|h| arena.get_node_exit_opr(&h));

        let mut sy = arena.take_sync_reg(self.sync_reg_i);
        for (src_node, cond) in dep_list {
            assert!(cond.is_none(), "SynNode dep nodes must not carry condition");
            let src_exit = arena.get_node_exit_opr(&src_node);
            sy.add_depend_node(src_exit, force_exit_inv);
        }
        if let Some(rs) = int_rst_sig     { sy.set_rst_sig_i(rs); }
        if let Some(fe) = force_exit_exit { sy.set_rst_sig_i(fe); }

        sy.build_support_signal(arena);
        sy.build_update_event(arena);
        let end_expr = sy.get_end_expr_i().expect("end_expr after build");
        arena.replace_back_sync_reg(sy);

        let bound = self.bind_with_rst_output_if_reset(arena, end_expr);
        self.end_expr_i   = Some(end_expr);
        self.bound_exit_i = Some(bound);
    }

    fn get_exit_opr      (&self) -> HcpIdent { self.bound_exit_i.or(self.end_expr_i).unwrap_or_default() }
    fn is_state_full_node(&self) -> bool      { false }
}

impl Identifiable for SynNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
