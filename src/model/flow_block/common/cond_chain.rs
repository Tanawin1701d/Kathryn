use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::add_logic_with_output;

/// Tracks the rolling `prev_false` signal through a mutual-exclusion condition chain
/// (if / elif* / else?). Callers drive the chain with `step` and use the returned
/// absolute condition for each branch however they need.
pub struct CondChain {
    prev_false: Option<HcpIdent>,
}

impl CondChain {
    /// Start the chain by inverting `cond_i` as the initial `prev_false`.
    /// `prefix` is prepended to generated expression names (e.g. `"cif"`, `"zcif"`).
    pub fn new_inv(arena: &mut ModelArena, prefix: &str, id: u64, cond_i: HcpIdent) -> Self {
        let inv = arena.make_expression_single(
            false, &format!("{}_not_cond_{}", prefix, id),
            LogicOp::BitwiseInvr, cond_i, None,
        );
        Self { prev_false: Some(inv) }
    }

    /// Advance one step in the chain for a continuation branch at index `i`.
    ///
    /// - `Some(elif_cond)` — elif branch: returns `elif_cond & prev_false` and advances
    ///   `prev_false` to `prev_false & ~elif_cond`.
    /// - `None` — else branch: consumes and returns `prev_false` (always the last step).
    pub fn step(
        &mut self,
        arena    : &mut ModelArena,
        prefix   : &str,
        id       : u64,
        i        : usize,
        elif_cond: Option<HcpIdent>,
    ) -> Option<HcpIdent> {
        match elif_cond {
            Some(c) => {
                let gated = add_logic_with_output(arena, Some(c), self.prev_false, LogicOp::BitwiseAnd);
                let inv_elif = arena.make_expression_single(
                    false, &format!("{}_not_elif_{}_{}", prefix, id, i),
                    LogicOp::BitwiseInvr, c, None,
                );
                self.prev_false = add_logic_with_output(arena, self.prev_false, Some(inv_elif), LogicOp::BitwiseAnd);
                gated
            }
            None => self.prev_false.take(),
        }
    }

    /// Returns the residual `prev_false` after all branches, for fall-through wiring.
    pub fn remaining_false(self) -> Option<HcpIdent> { self.prev_false }
}
