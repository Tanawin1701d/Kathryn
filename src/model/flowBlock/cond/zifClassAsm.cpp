//
// Created by tanawin on 27/11/25.
//

#include "zifClassAsm.h"


namespace kathryn{

    /**
 *
 * ZifClassAsm
 *
 ***/

    ZifClassAsm::ZifClassAsm(Operable* condition, AssignMeta* assignMeta):
    ClassAssignMeta(assignMeta),
    condition(condition){}


    void ZifClassAsm::addZelifStage(ZifClassAsm* classAsmMeta){
        ///// the ownnership of assignMeta in sub ZifClassAsm is still belong to them
        assert(isJoinable(classAsmMeta));
        subZelif.pushBack(classAsmMeta);

    }

    AsmNode* ZifClassAsm::createAsmNode(){

        UpdateEventGrp* mainGrp = createEventGrp();
        condUeEvent = new UpdateEventCond();
        condUeEvent->addSubStmt(condition, mainGrp);

        ///// sample should be the first one in the block
        AssignMeta* sampleMeta = getSampleAssignMetaPtr();
        UpdatePool* sampleEventPool = sampleMeta->getEventPoolPtr();
        ASM_TYPE    sampleAsmType   = sampleMeta->getAsmType();

        ///// add sub zelif ue


        for (ZifClassAsm* classAsmMeta: subZelif){
            condUeEvent->addSubStmt(classAsmMeta->condition, classAsmMeta->createEventGrp());
        }

        AssignMeta* poolAssMeta = new AssignMeta(condUeEvent, *sampleEventPool, sampleAsmType, sampleMeta->getCurAssignCnt());
        AsmNode* newAsmNode = new AsmNode(poolAssMeta);
        return newAsmNode;

    }


    void tryAddOrCreateAsmMeta(AsmNode* asmNode,
                          std::vector<ZifClassAsm*>& assignMetas,
                          Operable* purifiedCondition){
        assert(asmNode != nullptr);
        for (AssignMeta* asmMeta: asmNode->getAssignMetas()){
            bool found = false;
            for (ZifClassAsm* classAsm: assignMetas){
                if (classAsm->isJoinable(asmMeta)){
                    classAsm->addAssignMeta(asmMeta);
                    found = true;
                    break;
                }
            }
            if (!found){
                assignMetas.pushBack(new ZifClassAsm(purifiedCondition,asmMeta));
            }
        }
        asmNode->transferOutAssignMetaOwnership();

    }


}