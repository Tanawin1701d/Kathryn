use crate::model::controller::clock_mode::ClockMode;
use crate::model::controller::clock_mode::ClockMode::ClkUnused;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::UeGrp;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

// One group: all AssignMetas that target the same HCP with joinable UEs get merged into a UeGrp.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AssignMetaGrpGroup {
    target_hwc   : HcpIdent,             // HCP being assigned into
    update_events: Vec<UpdateEventIdent>, // individual UEs to merge into a single UeGrp
}

impl AssignMetaGrpGroup {
    /// Construct a group seeded with `first_ue`; the group grows as joinable UEs are pushed in.
    pub fn new(target_hwc: HcpIdent, first_ue: UpdateEventIdent) -> Self {
        Self {
            target_hwc,
            update_events: vec![first_ue],
        }
    }

    /// True when `asm` targets the same HCP and its UE is joinable with this group's reference UE.
    pub fn is_joinable(&self, arena: &mut ModelArena, asm: &AssignMeta) -> bool {
        self.target_hwc == asm.get_target_hw() &&
            self.update_events.first().copied()
                .map_or(false, |ref_ue|
                    arena.is_ue_joinable(ref_ue, asm.get_pre_update_event()))
    }

    /// Steal the first joinable candidate from `asm_candidates` and append it; returns true if one was found.
    pub fn try_push_event(&mut self, arena: &mut ModelArena,
                          asm_candidates: &mut Vec<AssignMeta>) -> bool {
        // before push_event, you should check with is_joinable
        assert!(!self.update_events.is_empty(), "update_events must be initialised before try_push_event");

        let pos = asm_candidates.iter().position(|asm_can| self.is_joinable(arena, asm_can));
        if let Some(idx) = pos {
            self.update_events.push(asm_candidates.remove(idx).get_pre_update_event());
        }
        pos.is_some()
    }

    // ---- reference-UE accessors (read from the first UE in group) ----

    fn get_ref_priority(&self, arena: &mut ModelArena) -> i32 {
        self.update_events.first().copied()
            .map_or(0, |ue| arena.get_ue_common(&ue).get_priority())
    }

    fn get_ref_clk_mode(&self, arena: &mut ModelArena) -> ClockMode {
        self.update_events.first().copied()
            .map_or(ClkUnused, |ue| arena.get_ue_common(&ue).get_clk_mode())
    }

    fn get_ref_clk_src_i(&self, arena: &mut ModelArena) -> Option<HcpIdent> {
        self.update_events.first().copied()
            .and_then(|ue| arena.get_ue_common(&ue).get_clk_src_i())
    }

    // ---- UE / AssignMeta generation ----

    /// Build a UeGrp from all accumulated UEs and insert it into the arena.
    pub fn gen_update_event(&self, arena: &mut ModelArena) -> UpdateEventIdent {
        let ue_len = self.update_events.len();
        assert!(ue_len >= 1, "update_events must have at least one entry (first_ue from construction)");

        let mut ueg = UeGrp::new();
        for &ue_i in &self.update_events {
            let priority   = self.get_ref_priority(arena);
            let clk_mode   = self.get_ref_clk_mode(arena);
            let clk_src_i  = self.get_ref_clk_src_i(arena);
            ueg.add_sub_stmt(ue_i, priority, clk_mode, clk_src_i);
        }
        arena.insert_ue_grp(ueg)
    }

    /// Wrap the generated UeGrp in an AssignMeta for the target HCP.
    pub fn gen_assign_meta(&self, arena: &mut ModelArena) -> AssignMeta {
        let ue_i = self.gen_update_event(arena);
        AssignMeta::new(self.target_hwc, ue_i, ClkUnused)
    }
}

impl Default for AssignMetaGrpGroup {
    fn default() -> Self {
        Self {
            target_hwc   : HcpIdent::default(),
            update_events: Vec::new(),
        }
    }
}

// Pool of group-groups; dispatches incoming AssignMetas into the correct group by joinability.
pub struct AssignMetaGrpPool {
    asm_pool: Vec<AssignMetaGrpGroup>,
}

impl Default for AssignMetaGrpPool {
    fn default() -> Self { Self { asm_pool: Vec::new() } }
}

impl AssignMetaGrpPool {
    /// Start a new group seeded from `asm`.
    pub fn add_new_group(&mut self, asm: &AssignMeta) {
        let group = AssignMetaGrpGroup::new(asm.get_target_hw(), asm.get_pre_update_event());
        self.asm_pool.push(group);
    }

    /// Route each candidate into an existing joinable group, or open a new one if none matches.
    pub fn insert_asms(&mut self, arena: &mut ModelArena, asm_candidates: &[AssignMeta]) {
        let mut candidates_copy = asm_candidates.to_vec();
        for group in &mut self.asm_pool {
            group.try_push_event(arena, &mut candidates_copy);
        }
        // Remaining candidates didn't fit any existing group — each becomes a new group.
        candidates_copy.iter()
            .for_each(|asm| self.add_new_group(asm));
    }

    /// Emit one AssignMeta per group, each backed by a merged UeGrp.
    pub fn gen_assign_metas(&self, arena: &mut ModelArena) -> Vec<AssignMeta> {
        self.asm_pool.iter()
            .map(|group| group.gen_assign_meta(arena))
            .collect()
    }
}
