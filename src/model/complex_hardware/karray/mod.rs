// Karray CCP. Re-export only — keeps `complex_hardware::karray::{Karray, ...}` paths.

pub mod karray;
pub mod arena_impl_ccp_karray;
pub mod karray_meta;
pub mod karray_region_sel;
pub mod karray_static_index;
pub mod karray_dyn_sel;
pub mod karray_dynamic_get;
pub mod karray_dynamic_assign;
pub mod karray_dynamic_cus_assign;
pub mod karray_dynamic_cus_assign_run;
pub mod karray_dynamic_reduce_get;
pub mod karray_dynamic_reduce_get_run;

pub use karray::{Karray, KARRAY_BACKINGS};
pub use karray_meta::{KarrayField, KarrayType};
pub use karray_region_sel::KarrayAsmErr;
pub use karray_dyn_sel::{DynSelKarray, KyIdxType};
pub use karray_dynamic_cus_assign::WriteDim;
pub use karray_dynamic_cus_assign_run::{write_run, WriteEnv};
pub use karray_dynamic_reduce_get::{reduce_mux, reduce_pack, NamedHcp, ReduceDim};
pub use karray_dynamic_reduce_get_run::{reduce_run, ReduceEnv};
