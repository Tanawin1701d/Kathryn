use std::sync::{OnceLock, Mutex};
use std::sync::atomic::{AtomicBool, Ordering};

pub mod flags;
pub mod config;

pub use flags::DebugFlag;
pub use config::{DebugBuilder, OutputMode};

// ---- FLAG_TABLE — one AtomicBool per DebugFlag variant; zero heap ----
//
// Hot path for every dprint!: one array index + Relaxed atomic load.
// No lock, no allocation.  The sink mutex is only touched when a flag is on.

const ATOMIC_FALSE: AtomicBool = AtomicBool::new(false);
pub(crate) static FLAG_TABLE: [AtomicBool; DebugFlag::COUNT] = [ATOMIC_FALSE; DebugFlag::COUNT];

// ---- DEBUG_SINK — locked only when a line is actually written ----

pub(crate) static DEBUG_SINK: OnceLock<Mutex<config::DebugSink>> = OnceLock::new();

// ---- public API ----

/// Return a fresh builder to configure and install the debug session.
/// Call exactly once, typically in main() before any other work.
///
/// ```
/// debug::init()
///     .flags(&[DebugFlag::Arena, DebugFlag::Verilog])
///     .to_file("logs/kathryn_debug.log")
///     .build();
/// ```
pub fn init() -> DebugBuilder { DebugBuilder::new() }

/// Hot-path flag check — single array index + Relaxed load, no allocation.
/// Called inside every dprint! expansion before any formatting occurs.
#[inline(always)]
pub fn is_flag_enabled(flag: DebugFlag) -> bool {
    FLAG_TABLE[flag as usize].load(Ordering::Relaxed)
}

/// Write one formatted debug line to the installed sink.
/// Only reached when the flag check in dprint! has already passed.
pub fn emit(flag: DebugFlag, file: &str, line: u32, msg: String) {
    if let Some(sink) = DEBUG_SINK.get() {
        sink.lock().unwrap().write_entry(flag, file, line, &msg);
    }
}

// ---- dprint! macro ----
//
// Accepts any DebugFlag expression — an inline path or a file-level const.
//
// Embedded-constant style (recommended for files with many dprint! calls):
//   const DBG: DebugFlag = DebugFlag::Arena;   // declared once at top of file
//   dprint!(DBG, "Inserted {} at {:?}", name, handle);
//
// Inline style:
//   dprint!(DebugFlag::Verilog, "Emitting wire {} width {}", name, w);
//
// Output format:
//   [ARENA    ]  src/model/arena_impl.rs:42  |  Inserted reg foo at 5:1

#[macro_export]
macro_rules! dprint {
    ($flag:expr, $($arg:tt)*) => {{
        if $crate::debug::is_flag_enabled($flag) {
            $crate::debug::emit($flag, file!(), line!(), format!($($arg)*));
        }
    }};
}
