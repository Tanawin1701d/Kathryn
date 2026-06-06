// Python-facing flow-block lifecycle ops. Mirrors the host
// `arena_impl_flow_block.rs` / `arena_impl.rs` build-stack helpers. Minimal
// open/close surface: push a block onto the init stack, finalize it back into
// its parent (or the module), and build its hardware.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::flow_block::flow_block_ident_py::PyFlowBlockIdent;

#[pymethods]
impl PyModelArena {
    // Open a flow block: push it onto the init stack so subsequent node/sub-block
    // construction attaches to it.
    fn initialize_flow_block(&mut self, block_i: PyFlowBlockIdent) {
        self.arena.push_flow_block_init_stack(block_i.into());
    }

    // Close the active flow block: pop it (asserting it matches `block_i`) and
    // attach it to the new stack top, or to the enclosing module if none.
    fn finalize_flow_block(&mut self, block_i: PyFlowBlockIdent) {
        self.arena.finalize_flow_block(block_i.into());
    }

    // Type of the nearest enclosing skeleton block (seq/par) on the init stack,
    // as a `FlowBlockType` IntEnum index (Parallel when none is found). The DSL
    // uses it to auto-open a matching skeleton inside complex blocks.
    fn get_last_skeleton_flow_block_type(&self) -> u32 {
        self.arena.get_last_skeleton_flow_block_type().to_index()
    }
}
