use std::fmt;

// ---- DebugFlag — category tags for dprint! calls ----
//
// Each variant maps to one slot in FLAG_TABLE (debug/mod.rs).
// To add a new category: append a variant, increment COUNT, add a Display arm.

#[repr(usize)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DebugFlag {
    ArenaFactory   = 0,   // make_* / mk_* factory calls
    ArenaImpl      = 1,   // arena insert / take / replace operations
    ModelHwc       = 2,   // hardware component construction and mutation
    ModelModule    = 3,   // module creation and trace-stack management
    ModelFlowBlock = 4,   // flow-block build and node wiring
    ModelNode      = 5,   // node construction and linking
    BackendBase    = 6,   // shared backend utilities (routing, IO wires, graph)
    BackendVerilog = 7,   // Verilog emit pipeline
    Miscellaneous  = 8,   // one-off diagnostics that don't fit elsewhere
}

impl DebugFlag {
    // Must stay in sync with the number of enum variants above.
    pub const COUNT: usize = 9;

    // All variants in declaration order; used by DebugBuilder::all_flags().
    pub const ALL: [DebugFlag; Self::COUNT] = [
        DebugFlag::ArenaFactory,
        DebugFlag::ArenaImpl,
        DebugFlag::ModelHwc,
        DebugFlag::ModelModule,
        DebugFlag::ModelFlowBlock,
        DebugFlag::ModelNode,
        DebugFlag::BackendBase,
        DebugFlag::BackendVerilog,
        DebugFlag::Miscellaneous,
    ];
}

impl fmt::Display for DebugFlag {
    // Fixed-width uppercase tags keep output columns aligned.
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let tag = match self {
            DebugFlag::ArenaFactory   => "ARENA_FACTORY  ",
            DebugFlag::ArenaImpl      => "ARENA_IMPL     ",
            DebugFlag::ModelHwc       => "MODEL_HWC      ",
            DebugFlag::ModelModule    => "MODEL_MODULE   ",
            DebugFlag::ModelFlowBlock => "MODEL_FLOWBLOCK",
            DebugFlag::ModelNode      => "MODEL_NODE     ",
            DebugFlag::BackendBase    => "BACKEND_BASE   ",
            DebugFlag::BackendVerilog => "BACKEND_VERILOG",
            DebugFlag::Miscellaneous  => "MISC           ",
        };
        write!(f, "{}", tag)
    }
}
