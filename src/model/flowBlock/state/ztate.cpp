//
// Created by tanawin on 10/1/26.
//

#include "ztate.h"
#include "model/controller/controller.h"


namespace kathryn{

FlowBlockZtate::FlowBlockZtate(Operable &identState):
            FlowBlockBase(ZTATE_BLOCK,{
                    {FLOW_ST_BASE_STACK},
                     FLOW_JO_EXT_FLOW,
                     true
             }),
            _identState(identState) {}

    FlowBlockZtate::~FlowBlockZtate() {}


    void FlowBlockZtate::addElementInFlowBlock(Node* node) {
        assert(false);
    }

    void FlowBlockZtate::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(subBlock->getFlowType() == ZCASE_BLOCK);
        /** call base function */
        FlowBlockBase::addSubFlowBlock(subBlock);

        auto* subBlockZcase = static_cast<FlowBlockZCase*>(subBlock);

        std::vector<ClassAssignMeta*> classAssMetas = subBlockZcase->getClassAssMetas();


        _caseMatchPool.push_back(subBlockZcase->getCaseId());

        //// try to each assignment to group

        for(ClassAssignMeta* caseClassAssMeta: classAssMetas){
            bool found = false;
            for(ZStateClassAsm* zstateClassAsm: _assignMetas){
                if (zstateClassAsm->isJoinable(caseClassAssMeta)){
                    zstateClassAsm->addCaseBlock(subBlockZcase->getCaseId(), caseClassAssMeta);
                    found = true;
                    break;
                }
            }
            if(!found){
                auto* newZStateClassAsm = new ZStateClassAsm(&_identState);
                _assignMetas.push_back(newZStateClassAsm);
                newZStateClassAsm->addCaseBlock(subBlockZcase->getCaseId(), caseClassAssMeta);
            }
        }

    }

    void FlowBlockZtate::addConFlowBlock(FlowBlockBase *fb){
        assert(false);

    }

    void FlowBlockZtate::addIntSignal(INT_TYPE type, Operable* signal){
        mfAssert(!isThereIntStart(), "start interrupt can start in zblock");
        mfAssert(!isThereIntRst(), "start interrupt can reset in zblock");
    }

    NodeWrap *FlowBlockZtate::sumarizeBlock() {
        assert(false);
    }

    void FlowBlockZtate::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZtate::onDetachBlock() {
        setLazyDelete();
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZtate::buildHwComponent() {
        assert(false);
    }

    std::string FlowBlockZtate::getMdDescribe() {
        assert(false);
    }

    void FlowBlockZtate::addMdLog(MdLogVal *mdLogVal) {
        assert(false);
    }

    void FlowBlockZtate::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockZtate::doPostFunction() {
        onDetachBlock();
    }

    std::vector<AsmNode*> FlowBlockZtate::extract(){

        ///// generate the node
        std::vector<AsmNode*> result;
        for (ZStateClassAsm* ztateClassAsm: _assignMetas){
            result.push_back(ztateClassAsm->createAsmNode(_caseMatchPool));
        }
        return result;

    }

}