use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::complex_hardware::karray::karray_dynamic_reduce_get::{NamedHcp, ReduceDim};
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// ===== The reduce driver =====================================================
//
// `reduce_run` is a recursive descent over the dimensions: a `Pin` dim fixes its index
// and recurses; a `Fold` dim fans out over its extent, recurses into each sub-region,
// then reduces the resulting child subtrees along that dim with its select fn. Folded
// dims collapse innermost-first, and no flat pool of every element is built.
//
// Everything that touches the arena or the user's select callback goes through the
// `ReduceEnv` trait (so the algorithm is PyO3-free and holds NO arena borrow). That is
// what makes the re-entrancy safe: the user's select fn builds its select expression
// with the arena (re-entering it), so the connector's `ReduceEnv` impl borrows the
// arena only inside each op (scoped) and calls `select` with no borrow held. The arena
// building blocks the impl wires up live in `karray_dynamic_reduce_get.rs`.

// ---- ReduceEnv — the arena/callback surface the algorithm runs against -------

/// The operations `reduce_run` performs, factored out so the algorithm stays
/// PyO3-free. The connector implements this over the Python arena: the arena ops
/// (`leaf`/`mux`/`const_index`/`pack`) each take a SCOPED borrow, while `select`
/// invokes the user's Python callback with NO borrow held — so the callback may
/// re-enter the arena to build its select expression.
pub trait ReduceEnv {
    type Err;

    /// Read one element's fields at the fully-pinned coordinate `coord`.
    fn gen_leaf(&mut self, coord: &[usize]) -> Result<Vec<NamedHcp>, Self::Err>;

    /// Run folded dimension `dim`'s select fn on a compared pair (`a`/`b` are the
    /// carried fields, `a_at`/`b_at` the covered coordinates). Returns the 1-bit
    /// select-left signal (true picks `a`) and any extra fields to carry onward.
    fn callback_user_select(&mut self,
                            dim   : usize,
                            a     : &[NamedHcp], a_at: &[Vec<usize>],
                            b     : &[NamedHcp], b_at: &[Vec<usize>],
                            level : u32) -> Result<(HcpIdent, Vec<NamedHcp>), Self::Err>;

    /// Mux `(name, a_hcp, b_hcp)` triples under `sel` into fresh wires (order kept).
    fn mux(&mut self, pairs: Vec<(String, HcpIdent, HcpIdent)>, sel: HcpIdent) -> Result<Vec<NamedHcp>, Self::Err>;

    /// A constant index value of `width` bits (a per-dim winner index seed).
    fn const_index(&mut self, width: i32, value: usize) -> Result<HcpIdent, Self::Err>;

    /// Pack the winner's fields into a scalar result Karray.
    fn pack_to_karray(&mut self, fields: Vec<NamedHcp>) -> Result<CcpIdent, Self::Err>;
}

// ---- the recursive algorithm ------------------------------------------------

// The fixed inputs of one reduce, threaded through the recursion by reference.
struct Plan<'a> {
    dim_sels     : &'a [ReduceDim],
    shape        : &'a [usize],
    field_names  : &'a [String],
    request_index: bool,
}

// A subtree under construction:
//   covered      — the element coordinates this subtree spans.
//   fields       — its current karray fields (muxed) plus any user extras, name-keyed.
//   coord_result — the winner's index signal for each already-reduced folded dim.
struct WorkNode {
    covered     : Vec<Vec<usize>>,
    fields      : Vec<NamedHcp>,
    coord_result: Vec<Option<HcpIdent>>,
}

impl WorkNode {
    // A fresh leaf: one covered element at `coord`, carrying its fields; no dim reduced.
    fn leaf(coord: Vec<usize>, fields: Vec<NamedHcp>, ndim: usize) -> WorkNode {
        WorkNode { covered: vec![coord], fields, coord_result: vec![None; ndim] }
    }
}

/// Drive the full nested reduce. `dim_sels` marks pin/fold per dimension (a folded
/// dim's select fn is identified by its dimension index in `env.select`). Folded dims
/// are reduced innermost-first (highest dim index first). Returns the winner Karray
/// and, when `request_index`, the winner's index signal per folded dim (ascending
/// dim order).
pub fn reduce_run<E: ReduceEnv>(
    env          : &mut E,
    dim_sels     : &[ReduceDim],
    shape        : &[usize],
    field_names  : &[String],
    request_index: bool,
) -> Result<(CcpIdent, Vec<HcpIdent>), E::Err> {
    // step1: bundle the fixed inputs the recursion threads through.
    let plan = Plan { dim_sels, shape, field_names, request_index };

    // step2: recurse from dim 0 down to the winner subtree (pin descends, fold reduces).
    let mut coord = Vec::with_capacity(dim_sels.len());
    let winner    = reduce_dim(env, &plan, 0, &mut coord)?;

    // step3: collect the winner's per-folded-dim index signals (only if requested).
    let coords_result = winner_coords(&winner, &folded_dims(dim_sels), request_index);

    // step4: pack the winner's fields into the scalar result Karray and return both.
    let result = env.pack_to_karray(winner.fields)?;
    Ok((result, coords_result))
}

// Resolve dimension `dim_idx`: a `Pin` fixes its index and recurses; a `Fold` fans out
// over its extent, recurses into each child, then reduces the children along this dim.
fn reduce_dim<E: ReduceEnv>(env: &mut E, plan: &Plan, dim_idx: usize, coord: &mut Vec<usize>)
    -> Result<WorkNode, E::Err> {
    let ndim = plan.dim_sels.len();
    if dim_idx == ndim {                                   // leaf: every dim is pinned
        let fields = env.gen_leaf(coord)?;
        return Ok(WorkNode::leaf(coord.clone(), fields, ndim));
    }
    match plan.dim_sels[dim_idx] {
        ReduceDim::Pin(i) => {
            coord.push(i);
            let node = reduce_dim(env, plan, dim_idx + 1, coord)?;
            coord.pop();
            Ok(node)
        }
        ReduceDim::Fold => {
            let dim_extent = plan.shape[dim_idx];
            let idx_width  = index_width_for(dim_extent);
            let mut children = Vec::with_capacity(dim_extent);
            for i in 0..dim_extent {
                coord.push(i);
                let mut child = reduce_dim(env, plan, dim_idx + 1, coord)?;
                coord.pop();
                if plan.request_index {                    // record this child's index in dim_idx
                    child.coord_result[dim_idx] = Some(env.const_index(idx_width, i)?);
                }
                children.push(child);
            }
            reduce_axis(env, plan, dim_idx, children)
        }
    }
}

// Balanced 2:1 reduction of one fold dimension's children with its select fn.
fn reduce_axis<E: ReduceEnv>(env: &mut E, plan: &Plan, dim: usize, children: Vec<WorkNode>) -> Result<WorkNode, E::Err> {
    let mut nodes = children;
    let mut level = 0u32;
    while nodes.len() > 1 {
        let mut next  = Vec::with_capacity(nodes.len().div_ceil(2));
        let mut pairs = nodes.into_iter();
        while let Some(a) = pairs.next() {
            match pairs.next() {
                Some(b) => next.push(mux_pair(env, plan, dim, level, a, b)?),
                None    => next.push(a),                  // odd node carried up unchanged
            }
        }
        nodes  = next;
        level += 1;
    }
    Ok(nodes.into_iter().next().expect("a folded dimension has at least one element"))
}

// Reduce one pair: ask dim `dim`'s select fn which side wins, then mux the karray
// fields and the resolved per-dim index signals (separately) under that select, layering
// the user's extras on top for the next level.
fn mux_pair<E: ReduceEnv>(env: &mut E, plan: &Plan, dim: usize, level: u32, a: WorkNode, b: WorkNode) -> Result<WorkNode, E::Err> {
    // step1: ask dim `dim`'s select fn which side wins, plus any extra wires to carry.
    let (sel, extras) = env.callback_user_select(dim, &a.fields, &a.covered, &b.fields, &b.covered, level)?;

    // step2: mux the karray fields under the select, then layer the user extras on top.
    let field_pairs: Vec<_> = plan.field_names.iter()
        .map(|name| (name.clone(), field_hcp(&a.fields, name), field_hcp(&b.fields, name)))
        .collect();
    let mut fields = env.mux(field_pairs, sel)?;
    apply_extras(&mut fields, extras);

    // step3: carry each already-resolved per-dim index signal forward, muxed under `sel`
    // (an unresolved dim — outer or pinned — stays None).
    let mut coord_result = vec![None; a.coord_result.len()];
    for d in 0..a.coord_result.len() {
        if let (Some(a_idx), Some(b_idx)) = (a.coord_result[d], b.coord_result[d]) {
            let muxed = env.mux(vec![(format!("ridx{d}"), a_idx, b_idx)], sel)?;
            coord_result[d] = Some(muxed[0].1);
        }
    }

    // step4: the merged node covers both children; assemble it.
    let mut covered = a.covered;
    covered.extend(b.covered);
    Ok(WorkNode { covered, fields, coord_result })
}

// ---- small helpers ----------------------------------------------------------

// Indices of the dimensions to fold (reduce over).
fn folded_dims(dim_sels: &[ReduceDim]) -> Vec<usize> {
    (0..dim_sels.len()).filter(|&d| dim_sels[d] == ReduceDim::Fold).collect()
}

// The winner's index signal per folded dim, in ascending dim order (empty unless requested).
fn winner_coords(winner: &WorkNode, fold_dims: &[usize], request_index: bool) -> Vec<HcpIdent> {
    if !request_index {
        return Vec::new();
    }
    let mut asc = fold_dims.to_vec();
    asc.sort_unstable();
    asc.iter().map(|&d| winner.coord_result[d].expect("request_index: missing winner index")).collect()
}

// Layer the user's extra fields onto a node's fields: an extra replaces a field of the
// same name, else it is appended. (Extras are user-computed merged values, not muxed.)
fn apply_extras(fields: &mut Vec<NamedHcp>, extras: Vec<NamedHcp>) {
    for (name, hcp) in extras {
        match fields.iter_mut().find(|(n, _)| *n == name) {
            Some(slot) => slot.1 = hcp,
            None       => fields.push((name, hcp)),
        }
    }
}

fn field_hcp(fields: &[NamedHcp], name: &str) -> HcpIdent {
    fields.iter().find(|(n, _)| n == name).expect("reduce: field not found in node").1
}
