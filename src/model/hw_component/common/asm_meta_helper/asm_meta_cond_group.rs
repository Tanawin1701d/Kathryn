use crate::model::controller::clock_mode::ClockMode;
use crate::model::controller::clock_mode::ClockMode::ClkUnused;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::UeCond;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AssignMetaIfGroup{
    target_hwc    : HcpIdent,
    cond_rel_vecs : Vec<HcpIdent>,
    cond_abs_vecs : Vec<HcpIdent>,
    update_events : Vec<Option<UpdateEventIdent>>,

}


impl AssignMetaIfGroup{
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

    pub fn is_joinable(&self,
                       arena : &mut ModelArena,
                       asm   : &AssignMeta) -> bool{

        self.target_hwc == asm.get_target_hw() &&
            self.update_events.iter()
                .find_map(|&ue| ue)
                .map_or(false, |ref_ue| arena.is_ue_joinable(ref_ue, asm.get_pre_update_event()))
    }

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


        if let Some(cond_i) = cond_rel_i {
            self.cond_rel_vecs.push(cond_i);
        }
        self.cond_abs_vecs.push(cond_abs_i);

        pos.is_some()
    }

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
            let priority = self.get_ref_priority(arena);
            let clk_mode = self.get_ref_clk_mode(arena);
            uec.add_sub_stmt(rel_cond_i, ue_i, priority, clk_mode);
        }
        arena.insert_ue_cond(uec)
    }

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

pub struct AssignMetaIfPool{
    asm_pool: Vec<AssignMetaIfGroup>
}

impl AssignMetaIfPool{

    pub fn add_new_group(&mut self, asm: &AssignMeta, cond_abs_i: HcpIdent) {
        let group = AssignMetaIfGroup::new(
            asm.get_target_hw(),
            cond_abs_i,
            asm.get_pre_update_event(),
        );
        self.asm_pool.push(group);
    }

    pub fn insert_asms(&mut self, arena         : &mut ModelArena,
                                 cond_rel_i    : Option<HcpIdent>,
                                 cond_abs_i    : HcpIdent,
                                 asm_candidates: &Vec<AssignMeta>) {
        let mut candidates_copy = asm_candidates.clone();


        for group in &mut self.asm_pool {
            group.try_push_event(arena, cond_rel_i, cond_abs_i, &mut candidates_copy);
        }
        candidates_copy.iter().for_each(|asm| self.add_new_group(asm, cond_abs_i));
    }

    pub fn gen_assign_metas(&self, arena: &mut ModelArena) -> Vec<AssignMeta> {
        self.asm_pool.iter()
            .map(|group| group.gen_assign_meta(arena))
            .collect()
    }



}