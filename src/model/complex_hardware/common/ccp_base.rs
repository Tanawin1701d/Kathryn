use crate::model::model_arena::ModelArena;

// ---- CcpBase trait ----------------------------------------------------------

/// Behaviour shared by every complex component property (CCP).  `build` wires
/// the CCP's internal hardware graph (its own wires/expressions); call once
/// after the CCP is fully configured.
pub trait CcpBase {
    fn build(&mut self, arena: &mut ModelArena);
}
