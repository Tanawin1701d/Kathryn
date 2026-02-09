//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_MODEL_FLOWBLOCK_COND_ZIFCLASSASM_H
#define SRC_MODEL_FLOWBLOCK_COND_ZIFCLASSASM_H

#include "model/flowBlock/abstract/flowBlock_Base.h"

namespace kathryn{

    /***
       *
       * zelif-zelse will detach as conjucntion block and will be extracted in zif
       * zif will extract at controller
       * */

    struct ZifClassAsm: ClassAssignMeta{
        ///// the event in classAssignMeta will be the assignment in the master zif block
        std::vector<ZifClassAsm*> subZelif;
        Operable*        condition   = nullptr;
        UpdateEventCond* condUeEvent = nullptr;

        /** assignMeta is the first sample assign meta data*/
        ZifClassAsm(Operable* condition, AssignMeta* assignMeta);
        /** create Event Group*/
        void addZelifStage  (ZifClassAsm* classAsmMeta); //// zelse as well
        /** build Node and condUe that conclude this event*/
        AsmNode* createAsmNode();

    };

    void tryAddOrCreateAsmMeta(
        AsmNode* asmNode,
        std::vector<ZifClassAsm*>& assignMetas,
        Operable* purifiedCondition = nullptr);
    //// incase the AssignMeta in AsmNode cannot join any ClassAsm
    ////  purifiedCond is the condition to set item

}

#endif //SRC_MODEL_FLOWBLOCK_COND_ZIFCLASSASM_H
