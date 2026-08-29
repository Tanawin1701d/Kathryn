use crate::model::complex_hardware::karray::kidx::KarrayErr;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// ===== KReadEnv — the read engine's arena/callback surface ====================
//
// A `CusRd` (reduce) dim calls the user's SELECT FN per 2:1 node while the tree
// builds; from Python that fn re-enters the arena. Rules the engine lives by:
// - NEVER hold a long-lived arena borrow — every touch goes through
//   `with_arena` (a SCOPED borrow).
// - Every select goes through `reduce_select`, called with NO borrow held.
// - The engine stays PyO3-free: the Python connector implements this trait
//   with scoped `borrow_mut`s; Rust-native `DirectKEnv` wraps `&mut ModelArena`.

pub trait KReadEnv {
    type Err: From<KarrayErr>;

    /// Run `f` under a scoped arena borrow (dropped on return).
    fn with_arena<R>(&mut self, f: impl FnOnce(&mut ModelArena) -> R) -> R;

    /// Run reduce dim `dim`'s select fn on a compared pair (`a`/`b` are the
    /// carried named fields, `a_covered`/`b_covered` the dim indices each side
    /// spans). Returns the 1-bit select signal (true picks `a`) plus any extra
    /// named fields to layer onto the merged node for the next level.
    fn reduce_select(
        &mut self,
        dim      : usize,
        a_fields : &[(String, HcpIdent)], a_covered: &[usize],
        b_fields : &[(String, HcpIdent)], b_covered: &[usize],
        level    : u32,
    ) -> Result<(HcpIdent, Vec<(String, HcpIdent)>), Self::Err>;
}

// ---- Rust-native env ---------------------------------------------------------

/// Direct env over a `&mut ModelArena`. Reduce dims are only drivable from the
/// Python DSL (there is no Rust-side select fn), so `reduce_select` errors.
pub struct DirectKEnv<'a> {
    pub arena: &'a mut ModelArena,
}

impl KReadEnv for DirectKEnv<'_> {
    type Err = KarrayErr;

    fn with_arena<R>(&mut self, f: impl FnOnce(&mut ModelArena) -> R) -> R { f(self.arena) }

    fn reduce_select(
        &mut self,
        dim      : usize,
        _a_fields: &[(String, HcpIdent)], _a_covered: &[usize],
        _b_fields: &[(String, HcpIdent)], _b_covered: &[usize],
        _level   : u32,
    ) -> Result<(HcpIdent, Vec<(String, HcpIdent)>), KarrayErr> {
        Err(KarrayErr::Value(format!(
            "reduce dim {dim}: a select fn is required — reduce reads are driven \
             through the Python connector")))
    }
}
