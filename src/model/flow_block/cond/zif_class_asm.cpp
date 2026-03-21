//
// Created by tanawin on 27/11/25.
//

#include "zif_class_asm.h"


namespace kathryn{

    /**
 *
 * ZifClassAsm
 *
 ***/

    ZifClassAsm::ZifClassAsm(Operable* condition, AssignMeta* assign_meta):
    ClassAssignMeta(assign_meta),
    condition(condition){}


    void ZifClassAsm::add_zelif_stage(ZifClassAsm* class_asm_meta){
        ///// the ownnership of assign_meta in sub ZifClassAsm is still belong to them
        assert(is_joinable(class_asm_meta));
        sub_zelif.push_back(class_asm_meta);

    }

    AsmNode* ZifClassAsm::create_asm_node(){

        UpdateEventGrp* main_grp = create_event_grp();
        cond_ue_event = new UpdateEventCond();
        cond_ue_event->add_sub_stmt(condition, main_grp);

        ///// sample should be the first one in the block
        AssignMeta* sample_meta = get_sample_assign_meta_ptr();
        UpdatePool* sample_event_pool = sample_meta->get_event_pool_ptr();
        ASM_TYPE    sample_asm_type   = sample_meta->get_asm_type();

        ///// add sub zelif ue


        for (ZifClassAsm* class_asm_meta: sub_zelif){
            cond_ue_event->add_sub_stmt(class_asm_meta->condition, class_asm_meta->create_event_grp());
        }

        AssignMeta* pool_ass_meta = new AssignMeta(cond_ue_event, *sample_event_pool, sample_asm_type, sample_meta->get_cur_assign_cnt());
        AsmNode* new_asm_node = new AsmNode(pool_ass_meta);
        return new_asm_node;

    }


    void try_add_or_create_asm_meta(AsmNode* asm_node,
                          std::vector<ZifClassAsm*>& assign_metas,
                          Operable* purified_condition){
        assert(asm_node != nullptr);
        for (AssignMeta* asm_meta: asm_node->get_assign_metas()){
            bool found = false;
            for (ZifClassAsm* class_asm: assign_metas){
                if (class_asm->is_joinable(asm_meta)){
                    class_asm->add_assign_meta(asm_meta);
                    found = true;
                    break;
                }
            }
            if (!found){
                assign_metas.push_back(new ZifClassAsm(purified_condition,asm_meta));
            }
        }
        asm_node->transfer_out_assign_meta_ownership();

    }


}