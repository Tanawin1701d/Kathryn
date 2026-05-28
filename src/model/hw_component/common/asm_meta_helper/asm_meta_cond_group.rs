use crate::model::controller::clock_mode::ClockMode;
use crate::model::controller::clock_mode::ClockMode::ClkUnused;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::UeCond;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

// One if/else-if group: all AssignMetas that share the same target HCP and condition chain.
// cond_rel_vecs holds relative conditions (the chain guard at each branch level);
// cond_abs_vecs holds absolute conditions (the full path from root) for reuse checks.
// update_events may contain None to represent an else-branch with no assignment.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AssignMetaIfGroup{
    target_hwc    : HcpIdent,                    // HCP being assigned into
    cond_rel_vecs : Vec<HcpIdent>,               // per-branch relative condition signals
    cond_abs_vecs : Vec<HcpIdent>,               // per-branch absolute condition paths
    update_events : Vec<Option<UpdateEventIdent>>, // None = else/no-op branch
}


impl AssignMetaIfGroup{
    /// Construct a group seeded with the first conditional branch.
    pub fn new(target_hwc   : HcpIdent,
               first_con_abs: HcpIdent,
               first_ue     : UpdateEventIdent) -> AssignMetaIfGroup{
        Self{
            target_hwc,
            cond_rel_vecs: vec![first_con_abs],
            cond_abs_vecs: vec![first_con_abs],
            update_events: vec![Some(first_ue)],
        }
    }

    /// True when `asm` targets the same HCP and its UE is joinable with this group's reference UE.
    pub fn is_joinable(&self,
                       arena : &mut ModelArena,
                       asm   : &AssignMeta) -> bool{

        self.target_hwc == asm.get_target_hw() &&
            self.update_events.iter()
                .find_map(|&ue| ue)
                .map_or(false, |ref_ue| arena.is_ue_joinable(ref_ue, asm.get_pre_update_event()))
    }

    /// Append the next branch; steals the first joinable candidate from `asm_candidates`, or pushes None.
    /// The UE slot is always pushed (even as None) to keep cond_rel_vecs and update_events in lock-step.
    pub fn try_push_event(&mut self,
                          arena        : &mut ModelArena,
                          cond_rel_i   : Option<HcpIdent>,
                          cond_abs_i   : HcpIdent,
                          asm_candidates: &mut Vec<AssignMeta>) -> bool {
        // before push_event, you should check with is_joinable
        assert_eq!(self.cond_rel_vecs.len(), self.update_events.len(), "the number of cond and ue should match");

        // you have to push the update event eventhough ue_i is none, to protect the condition chain
        let pos = asm_candidates.iter().position(|asm_can| self.is_joinable(arena, asm_can));
        let ue_i = pos.map(|idx| asm_candidates.remove(idx).get_pre_update_event());
        self.update_events.push(ue_i);

        // cond_rel_i is None for the unconditional else branch, which gets no guard pushed.
        if let Some(cond_i) = cond_rel_i {
            self.cond_rel_vecs.push(cond_i);
        }
        self.cond_abs_vecs.push(cond_abs_i);

        pos.is_some()
    }

    // ---- reference-UE accessors (read from first non-None UE in group) ----

    fn get_ref_priority(&self, arena: &mut ModelArena) -> i32 {
        self.update_events.iter()
            .find_map(|&ue| ue)
            .map_or(0, |ue| arena.get_ue_common(&ue).get_priority())
    }

    fn get_ref_clk_mode(&self, arena: &mut ModelArena) -> ClockMode {
        self.update_events.iter()
            .find_map(|&ue| ue)
            .map_or(ClkUnused, |ue| arena.get_ue_common(&ue).get_clk_mode())
    }

    fn get_ref_clk_src_i(&self, arena: &mut ModelArena) -> Option<HcpIdent> {
        self.update_events.iter()
            .find_map(|&ue| ue)
            .and_then(|ue| arena.get_ue_common(&ue).get_clk_src_i())
    }

    // ---- UE / AssignMeta generation ----

    /// Build a UeCond from all accumulated branches and insert it into the arena.
    /// ue_len may be rel_len+1 when an else branch (no condition) is appended last.
    pub fn gen_update_event(&self, arena: &mut ModelArena) -> UpdateEventIdent {

        let ue_len  = self.update_events.len();
        let rel_len = self.cond_rel_vecs.len();
        assert!(
            ue_len == rel_len || ue_len == rel_len + 1,
            "update_events length ({ue_len}) must equal cond_rel_vecs length ({rel_len}) or be exactly 1 more (else branch)"
        );
        let mut uec = UeCond::new();
        for (i, &ue_i) in self.update_events.iter().enumerate() {
            let rel_cond_i = self.cond_rel_vecs.get(i).copied();
            let priority   = self.get_ref_priority(arena);
            let clk_mode   = self.get_ref_clk_mode(arena);
            let clk_src_i  = self.get_ref_clk_src_i(arena);
            uec.add_sub_stmt(rel_cond_i, ue_i, priority, clk_mode, clk_src_i);
        }
        arena.insert_ue_cond(uec)
    }

    /// Wrap the generated UeCond in an AssignMeta for the target HCP.
    pub fn gen_assign_meta(&self, arena: &mut ModelArena) -> AssignMeta {
        let ue_i = self.gen_update_event(arena);
        AssignMeta::new(self.target_hwc, ue_i, ClkUnused) // clock mode argument will be unused
    }
}

impl Default for AssignMetaIfGroup{
    fn default() -> Self {
        Self{
            target_hwc   : HcpIdent::default(),
            cond_rel_vecs: Vec::new(),
            cond_abs_vecs: Vec::new(),
            update_events: Vec::new(),
        }
    }
}

// Pool of if-groups; dispatches incoming AssignMetas into the correct group by joinability.
pub struct AssignMetaIfPool {
    asm_pool: Vec<AssignMetaIfGroup>,
}

impl Default for AssignMetaIfPool {
    fn default() -> Self { Self { asm_pool: Vec::new() } }
}

impl AssignMetaIfPool {
    /// Start a new group seeded from `asm` under the given absolute condition signal.
    pub fn add_new_group(&mut self, asm: &AssignMeta, cond_abs_i: HcpIdent) {
        let group = AssignMetaIfGroup::new(
            asm.get_target_hw(),
            cond_abs_i,
            asm.get_pre_update_event(),
        );
        self.asm_pool.push(group);
    }

    /// Route each candidate into an existing joinable group, or open a new one if none matches.
    pub fn insert_asms(&mut self,
                       arena         : &mut ModelArena,
                       cond_rel_i    : Option<HcpIdent>,
                       cond_abs_i    : HcpIdent,
                       asm_candidates: &[AssignMeta]) {
        let mut candidates_copy = asm_candidates.to_vec();
        for group in &mut self.asm_pool {
            group.try_push_event(arena, cond_rel_i, cond_abs_i, &mut candidates_copy);
        }
        // Remaining candidates didn't fit any existing group — each becomes a new group.
        candidates_copy.iter().for_each(|asm| self.add_new_group(asm, cond_abs_i));
    }

    /// Emit one AssignMeta per group covering all its accumulated conditional branches.
    pub fn gen_assign_metas(&self, arena: &mut ModelArena) -> Vec<AssignMeta> {
        self.asm_pool.iter()
            .map(|group| group.gen_assign_meta(arena))
            .collect()
    }
}
