use std::fmt;

// ---- DebugFlag — category tags for dprint! calls ----
//
// Each variant maps to one slot in FLAG_TABLE (debug/mod.rs).
// To add a new category: append a variant, increment COUNT, add a Display arm.

#[repr(usize)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DebugFlag {
    Arena     = 0,   // arena insert / take / replace operations
    Factory   = 1,   // make_* / mk_* factory calls
    Module    = 2,   // module creation and trace-stack management
    Routing   = 3,   // cross-module IO routing + remap passes
    FlowBlock = 4,   // flow-block build and node wiring
    Verilog   = 5,   // Verilog backend emit
    Io        = 6,   // IoWire construction and reuse
}

impl DebugFlag {
    // Must stay in sync with the number of enum variants above.
    pub const COUNT: usize = 7;

    // All variants in declaration order; used by DebugBuilder::all_flags().
    pub const ALL: [DebugFlag; Self::COUNT] = [
        DebugFlag::Arena,
        DebugFlag::Factory,
        DebugFlag::Module,
        DebugFlag::Routing,
        DebugFlag::FlowBlock,
        DebugFlag::Verilog,
        DebugFlag::Io,
    ];
}

impl fmt::Display for DebugFlag {
    // Fixed-width uppercase tags keep output columns aligned.
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let tag = match self {
            DebugFlag::Arena     => "ARENA    ",
            DebugFlag::Factory   => "FACTORY  ",
            DebugFlag::Module    => "MODULE   ",
            DebugFlag::Routing   => "ROUTING  ",
            DebugFlag::FlowBlock => "FLOWBLOCK",
            DebugFlag::Verilog   => "VERILOG  ",
            DebugFlag::Io        => "IO       ",
        };
        write!(f, "{}", tag)
    }
}
