// Karray CCP. Re-export only — keeps `complex_hardware::karray::{Karray, ...}` paths.

pub mod karray;
pub mod arena_impl_ccp_karray_static;
pub mod arena_impl_ccp_karray_dynamic;
pub mod karray_meta;
pub mod karray_static_sel;
pub mod karray_hw_build;
pub mod karray_static_get;
pub mod karray_static_assign;
pub mod karray_dyn_sel;
pub mod karray_dynamic_get;
pub mod karray_dynamic_assign;
pub mod karray_dynamic_cus_assign;
pub mod karray_dynamic_cus_assign_run;
pub mod karray_dynamic_reduce_get;
pub mod karray_dynamic_reduce_get_run;

pub use karray::{Karray, KARRAY_BACKINGS};
pub use karray_meta::{KarrayField, KarrayType};
pub use karray_static_sel::{KarrayAsmErr, StaticRdWrDim};
pub use karray_dyn_sel::{DynSelKarray, DynRdWrDim, DynRdReduceDim, DynWrCusDim};
pub use karray_dynamic_cus_assign_run::{write_run, WriteEnv};
pub use karray_dynamic_reduce_get::{reduce_mux, reduce_pack, NamedHcp};
pub use karray_dynamic_reduce_get_run::{reduce_run, ReduceEnv};
