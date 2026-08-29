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
// How an update event / assign meta gets its clock source:
// - ClkFree           : no wiring — `clk_signal` stays None, the event is
//                       emitted combinationally wherever it lives.
// - PosEdge / NegEdge : needs a concrete source, filled in LATER:
//     1. user-level assignment (AssignMeta + UpdateEvent) — created with
//        clk_signal = None, wired during the enclosing flow block's build
//        (`FlowBlockBase::build_common_hw`, from the block's ext_trigger clk).
//     2. flow-block internal register (UpdateEvent only, no AssignMeta) —
//        wired when the owning node asks the register to build its update
//        event, from that node's trigger clk.