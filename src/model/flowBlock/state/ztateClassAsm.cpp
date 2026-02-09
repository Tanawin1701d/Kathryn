//
// Created by tanawin on 10/1/26.
//

#include "ztateClassAsm.h"

/**
 *
 * ZStsteClassAsm
 *
 */

namespace kathryn{

    AssignMeta* ZStateClassAsm::getSampleAssignMeta(){
        return _caseAssignMetas[0]->getSampleAssignMeta();
    }


    ZStateClassAsm::ZStateClassAsm(Operable* identifier):
    _identifier(identifier){
        assert(_identifier != nullptr);
    }

    int ZStateClassAsm::getMaxSupportSize() const{
        return 1 << _identifier->getOperableSlice().getSize();
    }

    bool ZStateClassAsm::isJoinable(ClassAssignMeta* sample){

        AssignMeta* ourSample  = getSampleAssignMeta();
        AssignMeta* yourSample = sample->getSampleAssignMeta();
        return ourSample->isJoinable(*yourSample);

    }

    void ZStateClassAsm::addCaseBlock(int caseIdent, ClassAssignMeta* classAssignMeta){
        assert((caseIdent >= -1) && (caseIdent < getMaxSupportSize()));

        if (!isEmpty()){
            assert(isJoinable(classAssignMeta));
        }
        _caseIdent.push_back(caseIdent);
        _caseAssignMetas.push_back(classAssignMeta);
    }

    AsmNode* ZStateClassAsm::createAsmNode(const std::vector<int>& globCaseIdents){

        stateUeEvent = new UpdateEventSwitch(_identifier);

        assert(_caseIdent.size() == getSize());
        int originIdx = 0;
        for (int i = 0; i < _caseIdent.size(); i++){
            while(_caseIdent[i] != globCaseIdents[originIdx]){
                stateUeEvent->addSubStmt(globCaseIdents[originIdx], nullptr);
                originIdx++;
            }
            stateUeEvent->addSubStmt(_caseIdent[i], _caseAssignMetas[i]->createEventGrp());
            originIdx++;
        }

        ///// sample should be the first one in the block
        AssignMeta* sampleMeta = getSampleAssignMeta();
        UpdatePool* sampleEventPool = sampleMeta->getEventPool();
        ASM_TYPE    sampleAsmType   = sampleMeta->getAsmType();

        AssignMeta* poolAssMeta = new AssignMeta(stateUeEvent, *sampleEventPool, sampleAsmType, sampleMeta->getCurAssignCnt());
        AsmNode*    newAsmNode  = new AsmNode(poolAssMeta);
        return newAsmNode;
    }
}
