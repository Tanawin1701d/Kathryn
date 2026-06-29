// Karray CCP. Re-export only — keeps `complex_hardware::karray::{Karray, ...}` paths.

pub mod karray;
pub mod arena_impl_ccp_karray;
pub mod karray_meta;
pub mod karray_region_sel;
pub mod karray_static_index;
pub mod karray_dynamic_index;

pub use karray::{Karray, KARRAY_BACKINGS};
pub use karray_meta::{KarrayField, KarrayType};
pub use karray_region_sel::KarrayAsmErr;
pub use karray_dynamic_index::{DynSelKarray, KyIdxType};
