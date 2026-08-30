use crate::model::complex_hardware::arb::arb_policy::ArbLockedChannel;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// ---- ArbLeaf ----------------------------------------------------------------

/// One arbitration client: its request/ack reference wires and its priority
/// (same scale as the UE priority — larger value wins).
pub struct ArbLeaf {
    req_wire_i : HcpIdent,
    ack_wire_i : HcpIdent,
    priority   : i32,
    ack_locked : bool,      // true → ack_wire_i is a const val 1; build must not drive it
}

impl ArbLeaf {
    /// Create a leaf with freshly-made 1-bit req/ack wires named off `base`/`idx`.
    pub(super) fn new(base: &str, idx: usize, priority: i32, arena: &mut ModelArena) -> Self {
        let req_wire_i = arena.make_wire(false, &format!("{}_REQ{}", base, idx), 1);
        let ack_wire_i = arena.make_wire(false, &format!("{}_ACK{}", base, idx), 1);
        Self { req_wire_i, ack_wire_i, priority, ack_locked: false }
    }

    /// Create a leaf with one side hard-tied to constant 1; the other side stays
    /// a freshly-made 1-bit wire.
    pub(super) fn new_locked(base: &str, idx: usize, priority: i32, channel: ArbLockedChannel, arena: &mut ModelArena) -> Self {
        let (req_wire_i, ack_wire_i) = match channel {
            ArbLockedChannel::Req => (
                arena.make_val (false, &format!("{}_REQ{}", base, idx), 1, 1),
                arena.make_wire(false, &format!("{}_ACK{}", base, idx), 1),
            ),
            ArbLockedChannel::Ack => (
                arena.make_wire(false, &format!("{}_REQ{}", base, idx), 1),
                arena.make_val (false, &format!("{}_ACK{}", base, idx), 1, 1),
            ),
        };
        Self { req_wire_i, ack_wire_i, priority, ack_locked: channel == ArbLockedChannel::Ack }
    }

    pub fn get_req_wire_i(&self) -> HcpIdent { self.req_wire_i }
    pub fn get_ack_wire_i(&self) -> HcpIdent { self.ack_wire_i }
    pub fn get_priority  (&self) -> i32      { self.priority   }
    pub fn is_ack_locked (&self) -> bool     { self.ack_locked }
}
