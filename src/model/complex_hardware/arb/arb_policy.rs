// Arb metadata enums — the policy knobs a caller picks before building an Arb.
// - variant_name / from_index are the cross-language bridge (see CLAUDE.md §7.4);
//   both are exhaustive so a new variant fails to compile until named here.

// ---- ArbSamePriPolicy -------------------------------------------------------

/// How to resolve a tie when several requesting leaves share the same priority.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ArbSamePriPolicy {
    AckOne,   // grant only the earliest leaf in vector order
    AckAll,   // grant every same-priority requester
    NotAck,   // grant none while a same-priority conflict exists
}

impl ArbSamePriPolicy {
    /// Stable variant name — surfaced to other languages. Exhaustive, so a new
    /// variant fails to compile until it is named here.
    pub fn variant_name(self) -> &'static str {
        match self {
            ArbSamePriPolicy::AckOne => "AckOne",
            ArbSamePriPolicy::AckAll => "AckAll",
            ArbSamePriPolicy::NotAck => "NotAck",
        }
    }

    /// Map a 0-based declaration-order index back to its variant; `None` past the last.
    pub fn from_index(idx: u32) -> Option<ArbSamePriPolicy> {
        match idx {
            0 => Some(ArbSamePriPolicy::AckOne),
            1 => Some(ArbSamePriPolicy::AckAll),
            2 => Some(ArbSamePriPolicy::NotAck),
            _ => None,
        }
    }
}

// ---- ArbLockedChannel -------------------------------------------------------

/// Which side of a leaf is hard-tied to constant 1 instead of being a wire.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ArbLockedChannel {
    Req,   // req tied to val 1 (leaf always requests); ack stays a normal wire
    Ack,   // ack tied to val 1 (leaf always granted);  req stays a normal wire
}

impl ArbLockedChannel {
    /// Stable variant name — surfaced to other languages. Exhaustive, so a new
    /// variant fails to compile until it is named here.
    pub fn variant_name(self) -> &'static str {
        match self {
            ArbLockedChannel::Req => "Req",
            ArbLockedChannel::Ack => "Ack",
        }
    }

    /// Map a 0-based declaration-order index back to its variant; `None` past the last.
    pub fn from_index(idx: u32) -> Option<ArbLockedChannel> {
        match idx {
            0 => Some(ArbLockedChannel::Req),
            1 => Some(ArbLockedChannel::Ack),
            _ => None,
        }
    }
}
