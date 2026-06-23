// Karray CCP. Re-export only — keeps `complex_hardware::karray::{Karray, ...}` paths.

pub mod karray;

pub use karray::{Karray, KarrayField, KarrayType, KARRAY_BACKINGS};
