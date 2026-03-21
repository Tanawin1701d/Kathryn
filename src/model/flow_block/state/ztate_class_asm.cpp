//
// Created by tanawin on 10/1/26.
//

#include "ztate_class_asm.h"

/**
 *
 * ZStsteClassAsm
 *
 */

namespace kathryn{

    AssignMeta* ZStateClassAsm::get_sample_assign_meta_ptr(){
        return _caseAssignMetas[0]->get_sample_assign_meta_ptr();
    }


    ZStateClassAsm::ZStateClassAsm(Operable* identifier):
    _identifier(identifier){
        assert(_identifier != nullptr);
    }

    int ZStateClassAsm::get_max_support_size() const{
        return 1 << _identifier->get_operable_slice().get_size();
    }

    bool ZStateClassAsm::is_joinable(ClassAssignMeta* sample){

        AssignMeta* our_sample  = get_sample_assign_meta_ptr();
        AssignMeta* your_sample = sample->get_sample_assign_meta_ptr();
        return our_sample->is_joinable(*your_sample);

    }

    void ZStateClassAsm::add_case_block(int case_ident, ClassAssignMeta* class_assign_meta){
        assert((case_ident >= -1) && (case_ident < get_max_support_size()));

        if (!is_empty()){
            assert(is_joinable(class_assign_meta));
        }
        _caseIdent.push_back(case_ident);
        _caseAssignMetas.push_back(class_assign_meta);
    }

    AsmNode* ZStateClassAsm::create_asm_node(const std::vector<int>& glob_case_idents){

        state_ue_event = new UpdateEventSwitch(_identifier);

        assert(_caseIdent.size() == get_size());
        int origin_idx = 0;
        for (int i = 0; i < _caseIdent.size(); i++){
            while(_caseIdent[i] != glob_case_idents[origin_idx]){
                state_ue_event->add_sub_stmt(glob_case_idents[origin_idx], nullptr);
                origin_idx++;
            }
            state_ue_event->add_sub_stmt(_caseIdent[i], _caseAssignMetas[i]->create_event_grp());
            origin_idx++;
        }

        ///// sample should be the first one in the block
        AssignMeta* sample_meta = get_sample_assign_meta_ptr();
        UpdatePool* sample_event_pool = sample_meta->get_event_pool_ptr();
        ASM_TYPE    sample_asm_type   = sample_meta->get_asm_type();

        AssignMeta* pool_ass_meta = new AssignMeta(state_ue_event, *sample_event_pool, sample_asm_type, sample_meta->get_cur_assign_cnt());
        AsmNode*    new_asm_node  = new AsmNode(pool_ass_meta);
        return new_asm_node;
    }
}
