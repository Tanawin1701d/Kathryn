//
// Created by tanawin on 1/12/25.
//

#include "pseudo.h"
#include "model/controller/controller.h"


namespace kathryn{

FlowBlockPseudo::FlowBlockPseudo(AsmNode* plainNode):
            FlowBlockBase(PSUEDO_BLOCK,
                  {
                  {FLOW_ST_BASE_STACK},
                  FLOW_JO_EXT_FLOW,
                  true
                  }),
            _plainNode(plainNode){

        assert(plainNode != nullptr);
        FlowBlockPseudo::onAttachBlock();
        FlowBlockBase::addElementInFlowBlock(plainNode);
        FlowBlockPseudo::onDetachBlock();
    }

    FlowBlockPseudo::~FlowBlockPseudo() {}


    void FlowBlockPseudo::addElementInFlowBlock(Node* node) {
        assert(false);
    }

    void FlowBlockPseudo::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockPseudo::addConFlowBlock(FlowBlockBase *fb){
        assert(false);
    }

    void FlowBlockPseudo::addIntSignal(INT_TYPE type, Operable* signal){
        mfAssert(!isThereIntStart(), "start interrupt can start in Pseudo block");
        mfAssert(!isThereIntRst(), "start interrupt can reset in Pseudo block");
    }

    NodeWrap *FlowBlockPseudo::sumarizeBlock() {
        assert(false);
    }

    void FlowBlockPseudo::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPseudo::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPseudo::buildHwComponent() {
        assert(false);
    }

    std::string FlowBlockPseudo::getMdDescribe() {
        assert(false);
    }

    void FlowBlockPseudo::addMdLog(MdLogVal *mdLogVal) {
        assert(false);
    }

    std::vector<AsmNode*> FlowBlockPseudo::extract(){
        return {_plainNode};
    }


}