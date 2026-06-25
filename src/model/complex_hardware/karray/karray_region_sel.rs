// Per-dimension selector math for Karray region assignment (consumed by
// arena_impl_ccp_karray.rs). Pure index arithmetic — no arena access.

// Error kind so the Python connector can pick TypeError vs ValueError.
pub enum KarrayAsmErr {
    Type (String),   // operator/backing mismatch (|= vs *=)
    Value(String),   // shape mismatch, bounds, or no matching fields
}

// One fully-resolved per-dimension selector: the half-open range [start, start+len)
// over that dimension. `is_range` keeps a result dimension (slice / trailing full
// dim); an int selector collapses it (is_range = false, len = 1).
pub(crate) struct DimSel {
    start   : usize,
    len     : usize,
    is_range: bool,
}

// Resolve raw (start, stop?, is_range) selectors against `shape`: validate the
// rank, clamp open/over-large stops to the dimension, append trailing dims as full
// ranges, and bounds-check. Returns one DimSel per dimension.
pub(crate) fn resolve_dim_selectors(
    shape: &[usize],
    sel  : &[(usize, Option<usize>, bool)],
) -> Result<Vec<DimSel>, KarrayAsmErr> {
    if sel.len() > shape.len() {
        return Err(KarrayAsmErr::Value(format!(
            "too many indices: got {} for a {}-D Karray", sel.len(), shape.len())));
    }
    let mut out = Vec::with_capacity(shape.len());
    for (dim_idx, &dim_sz) in shape.iter().enumerate() {
        let (start, stop, is_range) = match sel.get(dim_idx) {
            Some(&(s, st, r)) => (s, st.unwrap_or(dim_sz).min(dim_sz), r),
            None              => (0, dim_sz, true),   // trailing dim → full range
        };
        if start >= stop {
            return Err(KarrayAsmErr::Value(format!(
                "empty/invalid selector [{start}:{stop}) for dimension {dim_idx} of size {dim_sz}")));
        }
        out.push(DimSel { start, len: stop - start, is_range });
    }
    Ok(out)
}

// The result shape = the lengths of the range-kept dimensions, in order.
pub(crate) fn result_shape(sels: &[DimSel]) -> Vec<usize> {
    sels.iter().filter(|s| s.is_range).map(|s| s.len).collect()
}

// Row-major decode of a flat index into a multi-index over `shape`.
pub(crate) fn flat_to_multi_index(flat: usize, shape: &[usize]) -> Vec<usize> {
    let mut idx = vec![0usize; shape.len()];
    let mut rem = flat;
    for d in (0..shape.len()).rev() {
        idx[d] = rem % shape[d];
        rem   /= shape[d];
    }
    idx
}

// Map a result multi-index `k` (one entry per range dim) back to full integer
// indices over every dimension (int selectors stay fixed at their start).
pub(crate) fn offset_to_abs_indices(abs_sels: &[DimSel], offset_indices: &[usize]) -> Vec<usize> {
    let mut out = Vec::with_capacity(abs_sels.len());
    let mut j   = 0;
    for s in abs_sels {
        if s.is_range { out.push(s.start + offset_indices[j]); j += 1; }
        else          { out.push(s.start); }
    }
    out
}
