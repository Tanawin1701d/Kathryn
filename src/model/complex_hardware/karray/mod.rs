// Karray CCP — typed multi-dimensional array with ONE unified index type.
// Re-export only; the module split is:
//   karray.rs      — the arena-stored struct (fields, shape, Reg/Wire backing)
//   karray_meta.rs — element record types + index-width helper
//   kidx.rs        — the unified index (Static / Dyn / Range / CusWe / CusRd)
//   karray_env.rs  — KReadEnv (scoped arena + reduce-select callback) + DirectKEnv
//   karray_view.rs — KView, the shaped read result the write engine consumes
//   karray_read.rs / karray_write.rs — the ONLY two engines; k2k = read→write
//                    composition done by the proxies, not a third engine
//   karray_hw_build.rs — shared wiring primitives (muxes, write-enables, join)
//   arena_impl_ccp_karray.rs — ModelArena proxies + layout queries

pub mod karray;
pub mod karray_meta;
pub mod kidx;
pub mod karray_env;
pub mod karray_view;
pub mod karray_hw_build;
pub mod karray_read;
pub mod karray_write;
pub mod arena_impl_ccp_karray;

pub use karray::{Karray, KARRAY_BACKINGS};
pub use karray_env::{DirectKEnv, KReadEnv};
pub use karray_meta::{KarrayField, KarrayType};
pub use karray_view::{KView, KViewPairing};
pub use kidx::{KarrayErr, KIdx};
