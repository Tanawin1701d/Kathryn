use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ParSyncMode {
    AutoSync,
    NoSync,
}

#[derive(Clone, Debug)]
pub struct ParSchematic {
    sync_mode               : ParSyncMode,
    // main block
    basic_state_node_i      : Option<NcpIdent>, // use to build state (incase no subblock/ only pure assignment)
    node_wraps_of_sub_blocks: Vec<NodeWrap>,
    // syncer
    syn_node_i              : Option<NcpIdent>,
    pseudo_exit_node_i      : Option<NcpIdent>,
    cycle_used              : i32,
}

impl ParSchematic {
    pub fn new(sync_mode: ParSyncMode) -> Self {
        Self {
            sync_mode,
            // main block
            basic_state_node_i      : None,
            node_wraps_of_sub_blocks: Vec::new(),
            // syncer
            syn_node_i              : None,
            pseudo_exit_node_i      : None,
            cycle_used              : IN_CONSIST_CYCLE_USED,
        }
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        // manage the new assign block and simple state
        self.build_common(base, arena);
        // make sync logic
        let exit_i = match self.sync_mode {
            ParSyncMode::AutoSync => self.build_auto_sync_exit(base, arena),
            ParSyncMode::NoSync   => self.build_no_sync_exit(base, arena),
        };
        // build result node wrap
        self.build_result(base, exit_i)
    }

    fn build_common(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) {
        assert!(
            !base.get_basic_nodes_i().is_empty() || !base.get_sub_blocks_i().is_empty(),
            "parallel flow block has no element"
        );
        assert!(base.get_con_blocks_i().is_empty(), "parallel flow block does not support con blocks");

        // Split sub-blocks by join policy: only SubFlow children become NodeWraps
        // here; BasicNodeFlow children are folded into the unified AsmNode below.
        let (_basic_node_flow_sub_blocks_i, sub_flow_sub_blocks_i) = base.scan_sub_blocks_by_policy();

        // Merge every basic AsmNode and every BasicNodeFlow sub-block into one
        // ordered, per-target-unified AssignMeta list (this preserves the original
        // insertion order between direct nodes and basic-flow blocks), then build a
        // single clean AsmNode that hangs off the shared StateNode.
        let grp_asms = base.gen_unified_asm_meta_flat(arena);

        // intialize cycle determiner
        let mut cycle_det = NodeWrapCycleDet::new();

        // Group the unified AsmNode under one shared StateNode (skip if no asms).
        if !grp_asms.is_empty() {
            let id    = base.get_ident().get_global_id();
            let asm_i = arena.make_asm_node_many(&format!("par_asm_{}", id), &grp_asms);
            arena.init_node_trigger(asm_i, base.get_ext_trigger_node(), false);
            //arena.init_asm_node_clk_src(asm_i); you don't have to create clk source because the flow block give you already

            let state_i = arena.make_state_node(&format!("par_state_{}", id));
            arena.init_node_trigger(state_i, base.get_ext_trigger_node(), false);
            base.add_sys_node(state_i);
            self.basic_state_node_i = Some(state_i);
            cycle_det.add_cycle(arena.get_node_cycle_used(&state_i));
            arena.assign_ncp_node(state_i, true, false);

            // the unified asm node is the state's single slave
            arena.add_depend_node_to_ncp(asm_i, state_i, None);
            arena.assign_asm_from_state_node(asm_i);
        }

        // Resolve each SubFlow sub-block into a NodeWrap so we have its
        // entrance/exit nodes and cycle count available for wiring and cycle
        // detection below.  BasicNodeFlow children are NOT walked here — they
        // were already folded into basic_state_node_i above.
        self.node_wraps_of_sub_blocks = sub_flow_sub_blocks_i.iter()
            .map(|block_i| arena.summarize_flow_block(*block_i))
            .collect();

        // Determine the maximum cycle depth across all parallel paths.
        for wrap in &self.node_wraps_of_sub_blocks {
            cycle_det.add_cycle(wrap.get_cycle_used());
        }
        self.cycle_used = cycle_det.get_max_cycle_horizon();

        // If an external start signal exists, wire it as a depend into every
        // parallel entrance so all paths are gated behind the same trigger.
        if let Some(int_start_i) = base.get_int_node(ExtSigType::Start) {
            if let Some(state_i) = self.basic_state_node_i {
                arena.add_depend_node_to_ncp(state_i, int_start_i, None);
            }
            for wrap in &self.node_wraps_of_sub_blocks {
                wrap.add_dep_to_entrances(arena, int_start_i, None);
            }
            cycle_det.add_cycle(IN_CONSIST_CYCLE_USED);
        }
    }

    fn amount_of_paths(&self) -> usize {
        usize::from(self.basic_state_node_i.is_some()) + self.node_wraps_of_sub_blocks.len()
    }

    /// Every parallel path's `(exit_node, cycle_cost)` in one flat list:
    ///   - the shared basic_state_node (cycle = 1, since a StateNode advances one cycle)
    ///   - every SubFlow sub-block's NodeWrap (exit + reported cycle count)
    /// Used by both auto-sync and no-sync to drive sync-node fan-in and to pick
    /// a single exit node when one path's cycle equals the block's max cycle.
    fn path_exit_nodes(&self) -> Vec<(NcpIdent, i32)> {
        let mut nodes: Vec<(NcpIdent, i32)> = Vec::new();
        if let Some(state_i) = self.basic_state_node_i {
            nodes.push((state_i, 1));
        }
        nodes.extend(
            self.node_wraps_of_sub_blocks.iter()
                .map(|w| (w.get_exit_node_i(), w.get_cycle_used())));
        nodes
    }

    // ---- static exit decision ----

    /// Pick a single path-exit whose cycle count equals this block's overall
    /// `cycle_used`.  Returns None when `cycle_used` is inconsistent
    /// (negative sentinel) — caller must fall back to an explicit sync node.
    fn exit_matching_cycle(&self) -> Option<NcpIdent> {
        if self.cycle_used < 0 { return None; }
        self.path_exit_nodes().into_iter()
            .find_map(|(node_i, cycle)| if cycle == self.cycle_used { Some(node_i) } else { None })
    }

    fn any_exit(&self) -> Option<NcpIdent> {
        self.path_exit_nodes().into_iter().map(|(node_i, _)| node_i).next()
    }

    fn build_auto_sync_exit(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NcpIdent {

        // in this case we cannot statically determine the number of cycle usage
        if self.cycle_used == IN_CONSIST_CYCLE_USED && self.amount_of_paths() > 1 {
            let syn_i = arena.make_syn_node(
                &format!("par_syn_{}", base.get_ident().get_global_id()),
                self.amount_of_paths() as i32,
            );
            arena.init_node_trigger(syn_i, base.get_ext_trigger_node(), false);
            for (exit_i, _) in self.path_exit_nodes() {
                arena.add_depend_node_to_ncp(syn_i, exit_i, None);
            }
            base.add_sys_node(syn_i);
            arena.assign_ncp_node(syn_i, true, true);
            self.syn_node_i = Some(syn_i);
        }

        self.syn_node_i
            .or_else(|| self.exit_matching_cycle())
            .or_else(|| {
                assert_eq!(self.amount_of_paths(), 1, "parallel auto-sync cannot choose exit node");
                self.any_exit()
            })
            .expect("parallel block exit node was not found")
    }

    fn build_no_sync_exit(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NcpIdent {

        if let Some(exit_i) = self.exit_matching_cycle() {
            exit_i
        } else if self.amount_of_paths() == 1 {
            self.any_exit().expect("parallel block exit node was not found")
        } else {
            let pseudo_i = arena.make_pseudo_node("par_no_sync_exit", 1, LogicOp::BitwiseOr);
            arena.init_node_trigger(pseudo_i, base.get_ext_trigger_node(), false);
            for (exit_i, _) in self.path_exit_nodes() {
                arena.add_depend_node_to_ncp(pseudo_i, exit_i, None);
            }
            base.add_sys_node(pseudo_i);
            arena.assign_ncp_node(pseudo_i, true, true);
            self.pseudo_exit_node_i = Some(pseudo_i);
            pseudo_i
        }
    }

    fn build_result(&self, base: &FlowBlockBase, exit_i: NcpIdent) -> NodeWrap {
        let mut result = NodeWrap::new();
        if let Some(state_i) = self.basic_state_node_i {
            result.add_entrance_node_i(state_i);
        }
        for wrap in &self.node_wraps_of_sub_blocks {
            result.get_entrance_nodes_i_from(wrap);
        }
        result.set_exit_node_i(exit_i);
        result.set_cycle_used(self.cycle_used);
        result
    }
}
