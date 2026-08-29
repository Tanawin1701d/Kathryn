use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_env::KReadEnv;
use crate::model::complex_hardware::karray::karray_hw_build::{bin_layer_select_left, mux_into_wire};
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::complex_hardware::karray::karray_view::KView;
use crate::model::complex_hardware::karray::kidx::{check_kidx, KarrayErr, KIdx};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// ===== Karray READ engine =====================================================
//
// Resolve a selection (every dim collapses to ONE element) to one HCP per
// requested field.
// - all-Static selection -> the backing HCPs directly (ZERO extra hardware).
// - each runtime dim     -> fan out over its indices, fold with a balanced
//   2:1 tree (diagram below). Works for reg and wire backings.
// - carried fields ride the SAME tree — one fresh wire per field per node; a
//   reduce dim carries EVERY field (its select fn may compare any of them),
//   else only the requested ones.
// - arena borrows are scoped through the env (`with_arena`) and NEVER held
//   across `reduce_select` — the Python select fn may re-enter the arena to
//   build its select expression (see karray_env.rs).
//
// Balanced 2:1 fold of one runtime dim — e.g. 5 elements (each box = a
// ReadNode carrying ALL its fields; the odd node rides up unpaired):
//
//   e0     e1     e2     e3     e4
//    ╲     ╱       ╲     ╱      │
//    [ m01 ]       [ m23 ]      │     level 0 select: ~addr[0]           (Dyn)
//        ╲          ╱           │                     reduce_select(a,b,0) (CusRd)
//         [ m0123 ]             │     level 1 select: ~addr[1] / (a,b,1)
//              ╲________________│
//                    │
//                [ root ]             level 2 select: ~addr[2] / (a,b,2)
//
// - `Dyn`   — ONE shared select bit per level: `~addr[level]` picks the LEFT child.
// - `CusRd` — the user select fn runs PER PAIR with each side's carried fields
//   + covered indices; it returns pick-left plus optional extras layered onto
//   the merged node for the next level.

// One subtree of the fold: the carried named fields (muxed wires or, at a leaf,
// the backing HCPs) plus — for the reduce fold — the indices of the folding dim
// this subtree covers.
struct ReadNode {
    fields : Vec<(String, HcpIdent)>,
    covered: Vec<usize>,
}

impl Karray {
    /// Resolve a selection + field NAME to that field's (possibly muxed) HCP —
    /// the read target of `d[i][sig].field`.
    pub fn read_one_field<E: KReadEnv>(&self, sel: &[KIdx], field: &str, env: &mut E) -> Result<HcpIdent, E::Err> {
        // The DSL speaks field NAMES, the engine speaks positions — translate
        // first so a typo'd name fails before any hardware is built.
        let field_idx = self.field_index(field).ok_or_else(||
            E::Err::from(KarrayErr::Value(format!("Karray has no field '{field}'"))))?;
        // Validate the raw per-dim selectors against the shape (rank + bounds
        // checks). `E::Err::from` lifts the core KarrayErr into the env's error
        // type (PyErr under Python, KarrayErr natively).
        check_kidx(self.get_shape(), sel).map_err(E::Err::from)?;
        // Delegate to the shared multi-field engine with a ONE-entry field list;
        // the result vec is parallel to it, so [0] IS the requested field — the
        // backing HCP directly when all-static, else the mux/reduce tree's root.
        let hcps = self.read_field_hcps(sel, &[field_idx], env)?;
        Ok(hcps[0])
    }

    /// Resolve a selection to a KView over the requested fields — the k2k
    /// source entry (one HCP per field, wrapped for the write engine).
    pub(crate) fn read_view<E: KReadEnv>(
        &self,
        sel       : &[KIdx],
        field_idxs: &[usize],
        env       : &mut E,
    ) -> Result<KView, E::Err> {
        check_kidx(self.get_shape(), sel).map_err(E::Err::from)?;
        let hcps = self.read_field_hcps(sel, field_idxs, env)?;

        let fields: Vec<(String, i32)> = field_idxs.iter().map(|&fi| {
            let field = &self.get_fields()[fi];
            (field.get_name().to_string(), field.get_width())
        }).collect();
        Ok(KView::from_read(fields, hcps))
    }

    // Resolve a selection to one HCP per entry of `field_idxs` (parallel
    // order). Shared by the single-field read and the k2k source side.
    fn read_field_hcps<E: KReadEnv>(
        &self,
        sel       : &[KIdx],
        field_idxs: &[usize],
        env       : &mut E,
    ) -> Result<Vec<HcpIdent>, E::Err> {
        // A reduce dim's select fn may inspect ANY field, so carry the full
        // record whenever one is present; else carry just the requested fields.
        let has_reduce = sel.iter().any(|s| matches!(s, KIdx::CusRd));
        let carried: Vec<usize> = if has_reduce { (0..self.field_count()).collect() }
                                  else          { field_idxs.to_vec() };

        let mut coord = Vec::with_capacity(self.get_dim_count());
        let node = self.read_dim(0, &mut coord, sel, &carried, env)?;

        // Map the carried fields back to the requested order. Extras only
        // replace-in-place or append, so carried positions stay stable.
        let out = if has_reduce {
            field_idxs.iter().map(|&fi| node.fields[fi].1).collect()
        } else {
            node.fields.iter().map(|(_, hcp_i)| *hcp_i).collect()
        };
        Ok(out)
    }

    // Recursive resolution of dimension `dim_idx`: Static descends; Dyn/CusRd
    // fan out over every index and fold.
    fn read_dim<E: KReadEnv>(
        &self,
        // ---- iterating — advances per recursion level ----
        dim_idx: usize,
        coord  : &mut Vec<usize>,
        // ---- static — fixed for the whole walk ----
        sel    : &[KIdx],
        carried: &[usize],
        // ---- context — scoped arena + reduce-select callbacks ----
        env    : &mut E,
    ) -> Result<ReadNode, E::Err> {
        // ---- base case: every dim pinned -> the backing HCPs of this element ----
        if dim_idx == self.get_dim_count() {
            let fields = carried.iter().map(|&fi| {
                (self.get_fields()[fi].get_name().to_string(), self.element_hcp(coord, fi))
            }).collect();
            return Ok(ReadNode { fields, covered: Vec::new() });
        }

        let len = self.get_shape()[dim_idx];
        match &sel[dim_idx] {
            KIdx::CusWe(_) => Err(E::Err::from(KarrayErr::Value(
                "a custom write-enable index cannot select a read; on the read side \
                 a custom fn is a reduce select (fn(a, b, level) -> pick-a)".into()))),

            KIdx::Static(fixed_idx) => {
                coord.push(*fixed_idx);
                let node = self.read_dim(dim_idx + 1, coord, sel, carried, env)?;
                coord.pop();
                Ok(node)
            }

            KIdx::Dyn(sig_i) => {
                let sig_i = *sig_i;
                let need  = index_width_for(len);
                let got   = env.with_arena(|arena| arena.get_hw_bit_sz(&sig_i));
                if got < need {
                    return Err(E::Err::from(KarrayErr::Value(format!(
                        "dynamic index for dim {dim_idx} needs >= {need} bits to address {len} elements, got {got}"))));
                }
                let children = self.read_dim_fanout(dim_idx, coord, sel, carried, env)?;
                self.fold_mux_tree(dim_idx, sig_i, children, env)
            }

            KIdx::CusRd => {
                let mut children = self.read_dim_fanout(dim_idx, coord, sel, carried, env)?;
                for (i, child) in children.iter_mut().enumerate() {
                    child.covered = vec![i];   // this fold's coverage restarts per dim
                }
                self.fold_reduce_tree(dim_idx, children, env)
            }
        }
    }

    // Fan out over every index of dimension `dim_idx`, recursing to resolve each child.
    fn read_dim_fanout<E: KReadEnv>(
        &self,
        // ---- iterating — advances per recursion level ----
        dim_idx: usize,
        coord  : &mut Vec<usize>,
        // ---- static — fixed for the whole walk ----
        sel    : &[KIdx],
        carried: &[usize],
        // ---- context — scoped arena + reduce-select callbacks ----
        env    : &mut E,
    ) -> Result<Vec<ReadNode>, E::Err> {
        let len = self.get_shape()[dim_idx];
        let mut children = Vec::with_capacity(len);
        for i in 0..len {
            coord.push(i);
            let child = self.read_dim(dim_idx + 1, coord, sel, carried, env)?;
            coord.pop();
            children.push(child);
        }
        Ok(children)
    }

    // ---- Dyn: balanced mux fold, one shared `~addr[layer]` bit per level ----
    fn fold_mux_tree<E: KReadEnv>(
        &self,
        // ---- static — the folded dim + its binary address ----
        dim_idx  : usize,
        sig_i    : HcpIdent,
        // ---- iterating — halves every level until one node remains ----
        mut nodes: Vec<ReadNode>,
        // ---- context — scoped arena access ----
        env      : &mut E,
    ) -> Result<ReadNode, E::Err> {
        let base = self.get_ccp_ident().get_global_name().to_string();   // stem for the mux-wire names

        // One pass per tree level (see the fold diagram in the file header);
        // `layer` doubles as the address bit steering that level (see
        // bin_layer_select_left), so it starts at bit 0 (the LSB).
        let mut layer: u32 = 0;
        while nodes.len() > 1 {
            // ONE shared select for every mux on this level: `~addr[layer]` = pick left.
            let layer_sel = env.with_arena(|arena| bin_layer_select_left(sig_i, layer, arena));

            // Pair up this level's nodes left-to-right into the next (half-sized) level.
            let mut next = Vec::with_capacity(nodes.len().div_ceil(2));
            let mut iter = nodes.into_iter();
            let mut pos: u32 = 0;                 // node position in the level — naming only
            while let Some(left) = iter.next() {
                match iter.next() {
                    // Full pair: mux every carried field into fresh wires -> one merged node.
                    Some(right) => next.push(env.with_arena(|arena|
                        mux_fields_pair(&base, dim_idx, layer, pos, left, right, layer_sel, arena))),
                    // Odd one out — carried up unchanged; it pairs at a later level.
                    None        => next.push(left),
                }
                pos += 1;
            }
            nodes  = next;
            layer += 1;
        }

        // The loop leaves exactly one node: the tree's root (the resolved read).
        Ok(nodes.pop().expect("a Karray dimension has at least one element"))
    }

    // ---- CusRd: balanced reduce fold, user select fn per pair ----------------
    fn fold_reduce_tree<E: KReadEnv>(
        &self,
        // ---- static — the folded dim ----
        dim_idx  : usize,
        // ---- iterating — halves every level until one node remains ----
        mut nodes: Vec<ReadNode>,
        // ---- context — scoped arena + reduce-select callbacks ----
        env      : &mut E,
    ) -> Result<ReadNode, E::Err> {
        let base      = self.get_ccp_ident().get_global_name().to_string();
        let mut level = 0u32;
        while nodes.len() > 1 {
            let mut next = Vec::with_capacity(nodes.len().div_ceil(2));
            let mut iter = nodes.into_iter();
            let mut pos  = 0u32;
            while let Some(left) = iter.next() {
                match iter.next() {
                    Some(right) => {
                        // select fn runs with NO arena borrow held (it may re-enter).
                        let (sel_i, extras) = env.reduce_select(
                            dim_idx,
                            &left.fields , &left.covered,
                            &right.fields, &right.covered,
                            level)?;
                        let mut node = env.with_arena(|arena|
                            mux_fields_pair(&base, dim_idx, level, pos, left, right, sel_i, arena));
                        apply_extras(&mut node.fields, extras);
                        next.push(node);
                    }
                    None => next.push(left),          // odd one out — carried up unchanged
                }
                pos += 1;
            }
            nodes  = next;
            level += 1;
        }
        Ok(nodes.pop().expect("a Karray dimension has at least one element"))
    }
}

// Mux two subtrees under `select_left`: each carried field gets a fresh wire
// driven by an AssignMeta::mux UeCond (positional pairing — both subtrees carry
// the same field list). Node wires are named from the karray base + tree
// position (bounded length — no per-level name compounding).
fn mux_fields_pair(
    // ---- static — tree position (names the node wires) ----
    base       : &str,
    dim_idx    : usize,
    layer      : u32,
    pos        : u32,
    // ---- consumed — the merged pair + its select ----
    left       : ReadNode,
    right      : ReadNode,
    select_left: HcpIdent,
    // ---- sink — arena the mux wires are built into ----
    arena      : &mut ModelArena,
) -> ReadNode {
    // Pairing is POSITIONAL: both subtrees carry the same field list, so
    // left.fields[f] lines up with right.fields[f] (same name, same width).
    let mut fields = Vec::with_capacity(left.fields.len());
    let paired     = left.fields.into_iter().zip(right.fields);
    for (f, (l_field, r_field)) in paired.enumerate() {
        let (name, l_hcp_i) = l_field;
        let (_,    r_hcp_i) = r_field;   // right's name is the same — drop it

        // One fresh wire per field: `res_w = select_left ? left : right`.
        // Its name encodes the tree position — D<dim> L<layer> N<node> F<field> —
        // so names stay bounded instead of compounding per level.
        let width = arena.get_hw_bit_sz(&l_hcp_i);
        let res_w = arena.make_wire(false, &format!("{base}_D{dim_idx}L{layer}N{pos}F{f}_DMUX"), width);
        mux_into_wire(arena, res_w, l_hcp_i, r_hcp_i, select_left);
        fields.push((name, res_w));
    }

    // The merged node spans both halves' dim indices (reduce-fold bookkeeping;
    // stays empty on the mux fold, which never reads `covered`).
    let mut covered = left.covered;
    covered.extend(right.covered);
    ReadNode { fields, covered }
}

// Layer the select fn's extras onto a merged node: an extra replaces a carried
// field of the same name (keeping its position), else it is appended. Extras
// are user-computed merged values, not muxed.
fn apply_extras(fields: &mut Vec<(String, HcpIdent)>, extras: Vec<(String, HcpIdent)>) {
    for (name, hcp_i) in extras {
        match fields.iter_mut().find(|(n, _)| *n == name) {
            Some(slot) => slot.1 = hcp_i,
            None       => fields.push((name, hcp_i)),
        }
    }
}
