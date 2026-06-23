use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::model_arena::ModelArena;

// ---- CcpBase trait ----------------------------------------------------------

/// Behaviour shared by every complex component property (CCP).  Implemented by
/// each concrete CCP type so the arena can dispatch over `Box<dyn CcpBase>` with
/// a single match (in `take_ccp`); everything else is zero-match trait dispatch,
/// mirroring `FlowBlock` / `HcpBase`.
pub trait CcpBase {
    /// Wire the CCP's internal hardware graph (its own wires/expressions); call
    /// once after the CCP is fully configured.
    fn build(&mut self, arena: &mut ModelArena);

    /// Overwrite the stored `CcpIdent` (used after module stamping re-stamps it).
    fn set_ccp_ident(&mut self, ident: CcpIdent);

    /// Put this CCP back into its own typed arena slot — each type knows its slot,
    /// so callers replace with zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena);
}
