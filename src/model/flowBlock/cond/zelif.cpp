//
// Created by tanawin on 2/2/2567.
//

#include "zelif.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockZELIF::FlowBlockZELIF(Operable& cond):
            FlowBlockBase(ZELIF,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_CON_FLOW,
                                  false
                          }){

        curCond = purifyCondition(&cond);
    }

    FlowBlockZELIF::FlowBlockZELIF():
            FlowBlockBase(ZELSE,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_CON_FLOW,
                                  false
                          }){

        curCond = nullptr;
    }

    Operable *FlowBlockZELIF::getPurifiedCurCond() {
        return curCond;
    }

    FlowBlockZELIF::~FlowBlockZELIF() {
        //_basicNodes.clear();
    }

    void FlowBlockZELIF::addElementInFlowBlock(Node* node) {
        assert(node != nullptr);
        // if (curCond != nullptr) {
        //     assert(node->getNodeType() == ASM_NODE);
        //     ((AsmNode*)node)->addPreCondition(curCond, BITWISE_AND);
        // }
        // FlowBlockBase::addElementInFlowBlock(node);
        assert(node->getNodeType() == ASM_NODE);
        AsmNode* castedNode = (AsmNode*)node;
        tryAddOrCreateAsmMeta(castedNode, _assignMetas, curCond);
        addAbandonNode(node);
    }

    void FlowBlockZELIF::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZELIF::addConFlowBlock(FlowBlockBase *fb) {
        assert(false);
    }

    void FlowBlockZELIF::addIntSignal(INT_TYPE type, Operable* signal){
        mfAssert(!isThereIntStart(), "start interrupt can start in zblock");
        mfAssert(!isThereIntRst(), "start interrupt can reset in zblock");
    }

    NodeWrap* FlowBlockZELIF::sumarizeBlock() {assert(false);}

    void FlowBlockZELIF::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZELIF::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZELIF::buildHwComponent() {
        assert(false);
    }

    std::string FlowBlockZELIF::getMdDescribe() {
        assert(false);
    }

    void FlowBlockZELIF::addMdLog(MdLogVal *mdLogVal) {
        assert(false);
    }

    void FlowBlockZELIF::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockZELIF::doPostFunction() {
        onDetachBlock();
    }

    std::vector<ZifClassAsm*> FlowBlockZELIF::getClassAssMetas(){
        return _assignMetas;
    }

}
