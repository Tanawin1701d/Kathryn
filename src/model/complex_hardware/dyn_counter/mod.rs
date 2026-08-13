// DynCounter CCP — accumulate counter with a per-statement conditional add
// chain committed by `update`.  Re-export only.

pub mod dyn_counter;
pub mod arena_impl_ccp_dyn_counter;

pub use dyn_counter::DynCounter;
