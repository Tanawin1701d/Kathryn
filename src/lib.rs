// Library crate root. Holds every module so the same code backs both the
// native binary and the PyO3 extension built by maturin.

pub mod model;
pub mod common;
pub mod util;
pub mod params;
pub mod backends;
pub mod debug;

// Python bindings layer — the ONLY place in the crate that contains PyO3
// macros. Gated behind the `python` feature so the default build stays clean.
#[cfg(feature = "python")]
pub mod applications;
