use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AssignMetaIfGroup{
    target_hwc    : HcpIdent,
    cond_vecs     : Vec<HcpIdent>,
    update_events : Vec<UpdateEventIdent>,

}


impl AssignMetaIfGroup{
    pub fn new(target_hwc: HcpIdent,
               first_cond: HcpIdent,
               first_ue  : UpdateEventIdent) -> AssignMetaIfGroup{
        Self{
            target_hwc,
            cond_vecs    : vec![first_cond],
            update_events: vec![first_ue]
        }
    }

    pub fn is_joinable(&self,
                       arena     : &mut ModelArena,
                       asm       : &AssignMeta) -> bool{

        self.target_hwc == asm.get_target_hw() &&
            arena.is_ue_joinable(
                self.update_events[0],
                asm.get_pre_update_event())
    }

}

impl Default for AssignMetaIfGroup{
    fn default() -> Self {
        Self{
            target_hwc   : HcpIdent::default(),
            cond_vecs    : Vec::new(),
            update_events: Vec::new(),
        }
    }
}

pub struct AssignMetaIfPool{
    asm_pool: Vec<AssignMetaIfGroup>

    

}