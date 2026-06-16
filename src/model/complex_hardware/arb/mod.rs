// Arbiter CCP. Re-export only — keeps `complex_hardware::arb::{Arb, ...}` paths.

pub mod arb;

pub use arb::{Arb, ArbLeaf, ArbSamePriPolicy};
