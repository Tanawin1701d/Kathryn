use crate::model::hw_component::common::hcp_ident::HcpIdent;

// Selector + result carrier types for Karray dynamic indexing. The build logic that
// consumes them lives in `karray_dynamic_index.rs`.

// ---- KyIdxType — how a Karray element is selected ---------------------------

/// How a single Karray dimension/element is selected at access time. `Static` is
/// resolved at build time (compile-time `usize`); the two `Dyn*` variants carry a
/// runtime signal and differ only in how that signal encodes the element index:
/// `DynBin` is a binary-encoded address, `DynOneHot` is a one-hot select line.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum KyIdxType {
    Static   (usize),       // compile-time index
    DynBin   (HcpIdent),    // runtime binary-encoded address signal
    DynOneHot(HcpIdent),    // runtime one-hot select signal
}

impl KyIdxType {
    /// True for the two runtime (signal-driven) selectors.
    pub fn is_dynamic(&self) -> bool { !matches!(self, KyIdxType::Static(_)) }

    /// The compile-time index, or `None` for a dynamic selector.
    pub fn static_index(&self) -> Option<usize> {
        match self {
            KyIdxType::Static(i) => Some(*i),
            _                    => None,
        }
    }

    /// The runtime index signal, or `None` for a static selector.
    pub fn signal(&self) -> Option<HcpIdent> {
        match self {
            KyIdxType::DynBin(sig_i) | KyIdxType::DynOneHot(sig_i) => Some(*sig_i),
            KyIdxType::Static(_)                                   => None,
        }
    }
}

/// A dynamically-selected Karray element: the mux-output HCP for each selected
/// field (`immediate_hcps` parallel to `field_names`), plus the resolved
/// per-dimension coordinate (`result_indexed_i`, one entry per dimension — a
/// `Static` for a statically-pinned dim, a `DynBin` wire holding the chosen index
/// for a dynamically-selected dim). Fields are `pub(super)` so the build logic in
/// `karray_dynamic_index.rs` can construct/populate them.
#[derive(Clone, Debug, PartialEq, Eq, Default)]
pub struct DynSelKarray{
    pub(super) immediate_hcps   : Vec<HcpIdent>,
    pub(super) field_names      : Vec<String>,
    pub(super) result_indexed_i : Vec<KyIdxType>,
}

impl DynSelKarray {
    pub fn get_immediate_hcps  (&self) -> &Vec<HcpIdent>  { &self.immediate_hcps   }
    pub fn get_field_names      (&self) -> &Vec<String>    { &self.field_names      }
    pub fn get_result_indexed_i(&self) -> &Vec<KyIdxType> { &self.result_indexed_i }
}

// ---- internal reduction node ------------------------------------------------

// One node of the per-dimension mux reduction: the muxed data wire for each
// selected field, and the (optional) carried index value for this dimension.
// `oh_sel_i` is the one-hot path's running select line — the OR of every one-hot bit
// this subtree covers — carried up so each mux ORs just its two children (instead
// of re-OR-ing the whole covered set). `None` on the binary path (it selects on a
// per-level address bit, not a per-subtree OR). `pub(super)` so the build logic in
// `karray_dynamic_index.rs` can construct/read it.
pub(super) struct DynReduc {
    pub(super) data_i  : Vec<HcpIdent>,
    pub(super) idx_i   : Option<HcpIdent>,
    pub(super) oh_sel_i: Option<HcpIdent>,
}
