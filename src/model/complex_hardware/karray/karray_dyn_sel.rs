use crate::model::hw_component::common::hcp_ident::HcpIdent;

// Selector + result carrier types for Karray dynamic indexing, plus the per-dimension
// plan enums for the dynamic write (`DynWrCusDim`) and reduce (`DynRdReduceDim`) families.
// The build logic that consumes them lives in `karray_dynamic_get.rs`,
// `karray_dynamic_cus_assign*.rs`, and `karray_dynamic_reduce_get*.rs`.

// ---- DynRdWrDim — how a Karray element is selected ---------------------------

/// How a single Karray dimension/element is selected at access time. `Static` is
/// resolved at build time (compile-time `usize`); the two `Dyn*` variants carry a
/// runtime signal and differ only in how that signal encodes the element index:
/// `DynBin` is a binary-encoded address, `DynOneHot` is a one-hot select line.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum DynRdWrDim {
    Static   (usize),       // compile-time index
    DynBin   (HcpIdent),    // runtime binary-encoded address signal
    DynOneHot(HcpIdent),    // runtime one-hot select signal
}

impl DynRdWrDim {
    /// True for the two runtime (signal-driven) selectors.
    pub fn is_dynamic(&self) -> bool { !matches!(self, DynRdWrDim::Static(_)) }

    /// The compile-time index, or `None` for a dynamic selector.
    pub fn static_index(&self) -> Option<usize> {
        match self {
            DynRdWrDim::Static(i) => Some(*i),
            _                    => None,
        }
    }

    /// The runtime index signal, or `None` for a static selector.
    pub fn signal(&self) -> Option<HcpIdent> {
        match self {
            DynRdWrDim::DynBin(sig_i) | DynRdWrDim::DynOneHot(sig_i) => Some(*sig_i),
            DynRdWrDim::Static(_)                                   => None,
        }
    }
}

// ---- per-dimension plan enums (dynamic write / reduce) ----------------------

/// Per-dimension plan for `cus_dynamic_assign`: pin to one index, fan out over a
/// half-open sub-range `[start, stop)`, or spread over the whole dim. In every
/// fanned-out case the user's callback decides each element's write-enable.
/// Consumed by `karray_dynamic_cus_assign*.rs`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum DynWrCusDim {
    Pin   (usize),
    Range { start: usize, stop: Option<usize> },   // half-open [start, stop); None = dim end
    Spread,                                         // whole dim (== Range{0, None})
}

/// Per-dimension reduce plan: pin to one index, or fold (reduce over its extent).
/// Consumed by `karray_dynamic_reduce_get*.rs`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum DynRdReduceDim {
    Pin (usize),
    Fold,
}

/// The result of one dimension's selection — `dynamic_index_get_dim` returns a
/// `DynSelKarray` for each dimension level (a `DynReduc` is the *internal* node used
/// only while folding one dimension; a `DynSelKarray` is the resolved outcome carried
/// out of it). It holds the mux-output HCP for each selected field
/// (`intermediate_hcps` parallel to `field_names`), plus the resolved per-dimension
/// coordinate (`result_indices`, one entry per dimension — a `Static` for a
/// statically-pinned dim, a `DynBin` wire holding the chosen index for a
/// dynamically-selected dim). Fields are `pub(super)` so the build logic in
/// `karray_dynamic_get.rs` can construct/populate them.
#[derive(Clone, Debug, PartialEq, Eq, Default)]
pub struct DynSelKarray{
    pub(super) intermediate_hcps : Vec<HcpIdent>,
    pub(super) field_names       : Vec<String>,
    pub(super) result_indices    : Vec<DynRdWrDim>,
}

impl DynSelKarray {
    pub fn get_intermediate_hcps(&self) -> &Vec<HcpIdent>  { &self.intermediate_hcps }
    pub fn get_field_names      (&self) -> &Vec<String>    { &self.field_names       }
    pub fn get_result_indices   (&self) -> &Vec<DynRdWrDim> { &self.result_indices    }
}

// ---- internal reduction node ------------------------------------------------

// Internal to one dimension's reduction (NOT a per-dimension result — that is
// `DynSelKarray`). One node of the per-dimension 2:1 mux fold: the muxed data wire
// for each selected field, and the (optional) carried index value for this dimension.
// `oh_sel_i` is the one-hot path's running select line — the OR of every one-hot bit
// this subtree covers — carried up so each mux ORs just its two children (instead
// of re-OR-ing the whole covered set). `None` on the binary path (it selects on a
// per-level address bit, not a per-subtree OR). `pub(super)` so the build logic in
// `karray_dynamic_get.rs` can construct/read it.
pub(super) struct DynReduc {
    pub(super) data_i  : Vec<HcpIdent>,
    pub(super) idx_i   : Option<HcpIdent>,
    pub(super) oh_sel_i: Option<HcpIdent>,
}
