use std::sync::atomic::{AtomicU8, AtomicI32, Ordering};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ClockMode {
    PosEdge,
    NegEdge,
    ClkFree,
    ClkUnused,
    Amt,
}

impl ClockMode {
    fn to_u8(self) -> u8 {
        match self {
            Self::PosEdge   => 0,
            Self::NegEdge   => 1,
            Self::ClkFree   => 2,
            Self::ClkUnused => 3,
            Self::Amt       => 4,
        }
    }

    fn from_u8(v: u8) -> Self {
        match v {
            0 => Self::PosEdge,
            1 => Self::NegEdge,
            2 => Self::ClkFree,
            3 => Self::ClkUnused,
            _ => Self::Amt,
        }
    }

    pub fn assert_edge(self) -> Self {
        assert!(
            self == Self::PosEdge || self == Self::NegEdge,
            "ClockMode mismatch: expected PosEdge or NegEdge, got {:?}", self
        );
        self
    }
}

static GLOBAL_CLK_MODE: AtomicU8 = AtomicU8::new(0); // default: PosEdge

pub fn get_global_clk_mode() -> ClockMode {
    ClockMode::from_u8(GLOBAL_CLK_MODE.load(Ordering::Relaxed))
}

pub fn set_global_clk_mode(mode: ClockMode) {
    mode.assert_edge();
    GLOBAL_CLK_MODE.store(mode.to_u8(), Ordering::Relaxed);
}


// ---- Clock-signal policy ----------------------------------------------------
//
// ClkFree update events / assign metas need no wiring: their `clk_signal` stays
// `None` and the event is emitted combinationally wherever it lives.
//
// PosEdge / NegEdge events do need a concrete clock source.  Two cases:
//
//   1. User-level assignment (AssignMeta + its UpdateEvent).  Both are created
//      with `clk_signal = None`.  The signal is filled in during the enclosing
//      flow block's build phase (`FlowBlockBase::build_common_hw`), drawn from
//      the block's ext_trigger_node clk.
//
//   2. Flow-block internal registers (UpdateEvent only — no AssignMeta).  The
//      clock signal is set when the owning node asks the register to build its
//      update event; the source is again the node's trigger clk.