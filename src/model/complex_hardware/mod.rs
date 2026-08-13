// Complex component properties (CCPs): self-contained hardware gadgets that own
// their own wires/expressions, are not stamped into a module, and are not part
// of the flow graph.  Each carries a `CcpIdent` handle.

pub mod common;
pub mod arb;
pub mod karray;
pub mod dyn_counter;
pub mod arena_factory_ccp;
pub mod arena_impl_ccp;
