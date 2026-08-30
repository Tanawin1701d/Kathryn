use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::complex_hardware::arb::arb_leaf::ArbLeaf;
use crate::model::complex_hardware::arb::arb_policy::{ArbLockedChannel, ArbSamePriPolicy};
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::complex_hardware::common::ccp_base::CcpBase;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::add_logic;

// ---- Arb --------------------------------------------------------------------

/// Priority arbiter — a complex component property (CCP).
/// - A leaf raises `req`; it is granted on `ack` when requesting AND master
///   ack is asserted AND no higher-priority leaf also requests. Equal-priority
///   ties break per `same_pri_policy`.
/// - `master_req` = OR of every leaf request; `master_ack_src` = the single
///   global grant source.
/// - NOT stamped into a module — it owns its req/ack/master wires directly;
///   `build` wires the combinational arbitration graph.
pub struct Arb {
    ident             : CcpIdent         ,
    leaves            : Vec<ArbLeaf>     ,
    master_req_wire_i : HcpIdent         ,    // OR of every leaf req (1 bit)
    master_ack_src_i  : Option<HcpIdent> ,    // single source gating every grant
    user_hold_i       : Option<HcpIdent> ,    // optional 1-bit hold — freezes every grant while asserted
    user_reset_i      : Option<HcpIdent> ,    // optional 1-bit reset — clears every grant while asserted
    same_pri_policy   : ArbSamePriPolicy
}

impl Default for Arb {
    fn default() -> Self {
        Self {
            ident             : CcpIdent::new(CcpType::Arb, false, ""),
            leaves            : Vec::new(),
            master_req_wire_i : HcpIdent::default(),
            master_ack_src_i  : None,
            user_hold_i       : None,
            user_reset_i      : None,
            same_pri_policy   : ArbSamePriPolicy::AckOne,
        }
    }
}

impl Arb {
    pub fn new(is_user_com: bool, name: &str, policy: ArbSamePriPolicy, arena: &mut ModelArena) -> Self {
        let master_req_wire_i = arena.make_wire(false, &format!("{}_MREQ", name), 1);
        Self {
            ident             : CcpIdent::new(CcpType::Arb, is_user_com, name),
            leaves            : Vec::new(),
            master_req_wire_i,
            master_ack_src_i  : None,
            user_hold_i       : None,
            user_reset_i      : None,
            same_pri_policy   : policy,
        }
    }

    // ---- builder methods (call before building the arb) --------------------

    /// Add a leaf with its own freshly-created req/ack wires; returns its index.
    pub fn add_leaf(&mut self, priority: i32, arena: &mut ModelArena) -> usize {
        let idx  = self.leaves.len();
        let base = self.ident.get_ident_base().get_rel_name().to_string();
        self.leaves.push(ArbLeaf::new(&base, idx, priority, arena));
        idx
    }

    /// Add a leaf with one channel hard-tied to constant 1; returns its index.
    /// `ArbLockedChannel::Req` → leaf always requests; `Ack` → leaf always granted
    /// (its ack is a const val 1, so `build` leaves it undriven).
    pub fn add_leaf_locked(&mut self, priority: i32, channel: ArbLockedChannel, arena: &mut ModelArena) -> usize {
        let idx  = self.leaves.len();
        let base = self.ident.get_ident_base().get_rel_name().to_string();
        self.leaves.push(ArbLeaf::new_locked(&base, idx, priority, channel, arena));
        idx
    }

    /// Bind the single source that drives the master-ack gate (set once).
    pub fn set_master_ack_src(&mut self, src_i: HcpIdent, arena: &ModelArena) {
        assert!(self.master_ack_src_i.is_none(), "Arb::set_master_ack_src: master ack already set");
        assert_eq!(arena.get_hw_bit_sz(&src_i), 1, "Arb::set_master_ack_src: master ack source must be 1 bit");
        self.master_ack_src_i = Some(src_i);
    }

    /// Hard-tie the master-ack gate to constant 1 (set once) — declare that NO pip
    /// block masters this arb, so every grant is decided by leaf arbitration alone.
    /// A pip built on this arb afterwards fails the already-set assert by design.
    pub fn set_master_ack_locked(&mut self, arena: &mut ModelArena) {
        assert!(self.master_ack_src_i.is_none(), "Arb::set_master_ack_locked: master ack already set");
        let base = self.ident.get_ident_base().get_rel_name().to_string();
        self.master_ack_src_i = Some(arena.make_val(false, &format!("{}_MACK", base), 1, 1));
    }

    /// Bind the optional 1-bit hold signal (set once). The caller is responsible
    /// for resolving any slice down to a single bit before calling this.
    pub fn set_user_hold(&mut self, sig_i: HcpIdent, arena: &ModelArena) {
        assert!(self.user_hold_i.is_none(), "Arb::set_user_hold: user hold already set");
        assert_eq!(arena.get_hw_bit_sz(&sig_i), 1, "Arb::set_user_hold: hold signal must be 1 bit");
        self.user_hold_i = Some(sig_i);
    }

    /// Bind the optional 1-bit reset signal (set once). The caller is responsible
    /// for resolving any slice down to a single bit before calling this.
    pub fn set_user_reset(&mut self, sig_i: HcpIdent, arena: &ModelArena) {
        assert!(self.user_reset_i.is_none(), "Arb::set_user_reset: user reset already set");
        assert_eq!(arena.get_hw_bit_sz(&sig_i), 1, "Arb::set_user_reset: reset signal must be 1 bit");
        self.user_reset_i = Some(sig_i);
    }

    // ---- accessors ---------------------------------------------------------
    pub fn get_ccp_ident        (&self)             -> CcpIdent          { self.ident }
    pub fn get_ccp_ident_mut    (&mut self)         -> &mut CcpIdent     { &mut self.ident }
    pub fn get_master_req_wire_i(&self)             -> HcpIdent          { self.master_req_wire_i }
    pub fn get_master_ack_src_i (&self)             -> Option<HcpIdent>  { self.master_ack_src_i  }
    pub fn get_user_hold_i      (&self)             -> Option<HcpIdent>  { self.user_hold_i       }
    pub fn get_user_reset_i     (&self)             -> Option<HcpIdent>  { self.user_reset_i      }
    pub fn leaf_count           (&self)             -> usize             { self.leaves.len() }
    pub fn get_leaf             (&self, idx: usize) -> &ArbLeaf          { &self.leaves[idx] }
    pub fn get_leaf_req_wire_i  (&self, idx: usize) -> HcpIdent          { self.leaves[idx].get_req_wire_i() }
    pub fn get_leaf_ack_wire_i  (&self, idx: usize) -> HcpIdent          { self.leaves[idx].get_ack_wire_i() }

    // ---- build helpers -----------------------------------------------------

    /// Drive `wire_i` from `src_i` via an internal combinational update event.
    fn drive_wire(arena: &mut ModelArena, wire_i: HcpIdent, src_i: HcpIdent) {
        //// we don't have to check size because for this node, we are so sure that it equal to 1
        let mut wire = arena.take_wire(wire_i);
        wire.bind_src(src_i, None, Slice::new(0,1), None, None, None, arena);
        arena.replace_back_wire(wire);
    }

    /// `ack[idx] = req[idx] & master_ack & ~higher_pri_reqs & same_pri_gate`.
    /// Higher-priority requests block the grant; same-priority ties are resolved
    /// by `same_pri_policy`.  Returns the assembled ack operand.
    fn build_leaf_ack(&self, arena: &mut ModelArena, idx: usize) -> HcpIdent {
        let leaf_pri = self.leaves[idx].get_priority();
        let mut ack_res_i = self.leaves[idx].get_req_wire_i();

        // gate on the global grant (its single source, when bound)
        if let Some(mack) = self.master_ack_src_i {
            ack_res_i = arena.make_expression(false, "arb_ack_mack", LogicOp::BitwiseAnd, ack_res_i, mack, None, None);
        }

        // any higher-priority requester blocks this leaf
        let mut higher_req_i: Option<HcpIdent> = None;
        for other_leaf in &self.leaves {
            if other_leaf.get_priority() > leaf_pri {
                add_logic(arena, &mut higher_req_i, other_leaf.get_req_wire_i(), LogicOp::BitwiseOr);
            }
        }
        if let Some(h) = higher_req_i {
            let higher_free_i = arena.make_expression_single(false, "arb_hi_inv", LogicOp::BitwiseInvr, h, None);
            ack_res_i = arena.make_expression(false, "arb_ack_hi", LogicOp::BitwiseAnd, ack_res_i, higher_free_i, None, None);
        }

        // same-priority resolution
        let mut same_req_i: Option<HcpIdent> = None;
        for (j, other_leaf) in self.leaves.iter().enumerate() {
            if other_leaf.get_priority() != leaf_pri { continue; }
            let blocks = match self.same_pri_policy {
                ArbSamePriPolicy::AckAll => false,      // never block an equal peer
                ArbSamePriPolicy::AckOne => j < idx,    // an earlier peer wins the tie
                ArbSamePriPolicy::NotAck => j != idx,   // any other peer cancels the grant
            };
            if blocks {
                add_logic(arena, &mut same_req_i, other_leaf.get_req_wire_i(), LogicOp::BitwiseOr);
            }
        }
        if let Some(s) = same_req_i {
            let same_free_i = arena.make_expression_single(false, "arb_same_inv", LogicOp::BitwiseInvr, s, None);
            ack_res_i = arena.make_expression(false, "arb_ack_same", LogicOp::BitwiseAnd, ack_res_i, same_free_i, None, None);
        }

        ack_res_i
    }
}

impl CcpBase for Arb {
    /// Wire the combinational arbitration graph: drive `master_req` and every
    /// leaf `ack`.  Call once after all leaves and the master-ack source are set.
    fn build(&mut self, arena: &mut ModelArena) {
        // master_req = OR of every leaf request
        let mut master_req_i: Option<HcpIdent> = None;
        for leaf in &self.leaves {
            add_logic(arena, &mut master_req_i, leaf.get_req_wire_i(), LogicOp::BitwiseOr);
        }
        if let Some(r) = master_req_i {
            Self::drive_wire(arena, self.master_req_wire_i, r);
        }
        // per-leaf ack arbitration (locked-ack leaves are const 1 — never driven)
        for idx in 0..self.leaves.len() {
            if self.leaves[idx].is_ack_locked() { continue; }
            let ack_res_i  = self.build_leaf_ack(arena, idx);
            let ack_wire_i = self.leaves[idx].get_ack_wire_i();
            Self::drive_wire(arena, ack_wire_i, ack_res_i);
        }
    }
    fn set_ccp_ident(&mut self, ident: CcpIdent) { *self.get_ccp_ident_mut() = ident; }
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_arb(*self); }
}

impl Identifiable for Arb {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}
