//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_MODEL_FLOWBLOCK_COND_ZIFCLASSASM_H
#define SRC_MODEL_FLOWBLOCK_COND_ZIFCLASSASM_H

#include "model/flow_block/abstract/flow_block__base.h"

namespace kathryn{

    /***
       *
       * zelif-zelse will detach as conjucntion block and will be extracted in zif
       * zif will extract at controller
       * */

    struct ZifClassAsm: ClassAssignMeta{
        ///// the event in class_assign_meta will be the assignment in the master zif block
        std::vector<ZifClassAsm*> sub_zelif;
        Operable*        condition   = nullptr;
        UpdateEventCond* cond_ue_event = nullptr;

        /** assign_meta is the first sample assign meta data*/
        ZifClassAsm(Operable* condition, AssignMeta* assign_meta);
        /** create Event Group*/
        void add_zelif_stage  (ZifClassAsm* class_asm_meta); //// zelse as well
        /** build Node and cond_ue that conclude this event*/
        AsmNode* create_asm_node();

    };

    void try_add_or_create_asm_meta(
        AsmNode* asm_node,
        std::vector<ZifClassAsm*>& assign_metas,
        Operable* purified_condition = nullptr);
    //// incase the AssignMeta in AsmNode cannot join any ClassAsm
    ////  purified_cond is the condition to set item

}

#endif //SRC_MODEL_FLOWBLOCK_COND_ZIFCLASSASM_H
