use crate::model::hw_component::common::hcp_ident::HcpIdent;

// ===== KIdx — the ONE unified Karray index type ===============================
//
// Every Karray access — read, write, and karray-to-karray copy — selects each
// dimension with a `KIdx`, and EVERY kind collapses its dimension to a single
// (possibly runtime-chosen) element: a selection always names exactly one
// element, and every dimension must be indexed. This file is the single home of
// the index kinds and their validation; the read/write engines
// (`karray_read.rs` / `karray_write.rs`) consume the checked selection
// directly. Adding an index kind touches exactly: one variant here, one arm in
// `check_kidx`, one arm in each engine.
//
//   KIdx::Static(i)   a[3]         compile-time index
//   KIdx::Dyn(sig)    a[addr]      runtime binary address
//   KIdx::CusWe(bits) a[fn]  WRITE one 1-bit enable per index
//   KIdx::CusRd       a[fn]  READ  reduce fold, fn picks pairs
//
// The DSL's single "custom fn" kind splits by DIRECTION at encode time:
//   * write destination — the fn is called once per index (`fn(i) -> 1-bit
//     signal`) and the pre-evaluated bits arrive as `CusWe`; each index's bit
//     gates that element's write enable.
//   * read / source side — the dim folds through a REDUCE tree: the fn is a
//     pair-select (`fn(a, b, level) -> pick-a`) called per 2:1 node while the
//     tree builds, so it cannot be pre-evaluated; `CusRd` marks the dim and the
//     select callback rides in via the `KReadEnv` (see karray_env.rs).

// ---- error ------------------------------------------------------------------

// Error kind so the Python connector can pick TypeError vs ValueError.
pub enum KarrayErr {
    Type (String),   // operator/backing mismatch (|= vs *=, dynamic write on wire)
    Value(String),   // rank / bounds / field errors
}

// ---- the four index kinds ---------------------------------------------------

/// One per-dimension selector, exactly as the user wrote it; the engines
/// consume it directly once `check_kidx` has validated the whole selection.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum KIdx {
    Static(usize),        // a[i]        pin one element at compile time
    Dyn   (HcpIdent),     // a[sig]      runtime binary-encoded address
    CusWe (Vec<HcpIdent>),// a[fn] write: one 1-bit enable per index of the dim
    CusRd,                // a[fn] read : reduce fold, select fn via KReadEnv
}

impl KIdx {
    pub(crate) fn is_runtime(&self) -> bool {
        matches!(self, KIdx::Dyn(_) | KIdx::CusWe(_) | KIdx::CusRd)
    }
}

// Validate raw selectors against `shape`: every dimension must be indexed (rank
// check both ways), static indices are bounds-checked, and a `CusWe` must carry
// exactly one bit per index of its dimension.
pub(crate) fn check_kidx(shape: &[usize], sel: &[KIdx]) -> Result<(), KarrayErr> {
    if sel.len() != shape.len() {
        return Err(KarrayErr::Value(format!(
            "a Karray selection must index every dimension: got {} indices for a {}-D Karray",
            sel.len(), shape.len())));
    }
    for (dim_idx, (&dim_sz, kidx)) in shape.iter().zip(sel.iter()).enumerate() {
        match kidx {
            KIdx::Static(i) => {
                if *i >= dim_sz {
                    return Err(KarrayErr::Value(format!(
                        "index {i} out of bounds for dimension {dim_idx} of size {dim_sz}")));
                }
            }
            KIdx::Dyn(_) => {}
            KIdx::CusWe(bits) => {
                if bits.len() != dim_sz {
                    return Err(KarrayErr::Value(format!(
                        "custom write index for dimension {dim_idx} needs one enable bit per index \
                         ({dim_sz}), got {}", bits.len())));
                }
            }
            KIdx::CusRd => {}
        }
    }
    Ok(())
}

// ---- selection queries ------------------------------------------------------

// True when any dim is collapsed at runtime (Dyn or Cus).
pub(crate) fn has_runtime(sel: &[KIdx]) -> bool {
    sel.iter().any(|s| s.is_runtime())
}
