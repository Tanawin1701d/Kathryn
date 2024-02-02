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

        curCond = &cond;
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

    Operable *FlowBlockZELIF::getCurCond() {
        return curCond;
    }

    FlowBlockZELIF::~FlowBlockZELIF() {
        basicNodes.clear();
        FlowBlockBase::~FlowBlockBase();
    }

    void FlowBlockZELIF::addElementInFlowBlock(Node *node) {
        assert(node != nullptr);
        if (curCond != nullptr)
            node->addCondtion(curCond, BITWISE_AND);
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockZELIF::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZELIF::addConFlowBlock(FlowBlockBase *fb) {
        assert(false);
    }

    NodeWrap* FlowBlockZELIF::sumarizeBlock() {}

    void FlowBlockZELIF::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZELIF::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZELIF::buildHwComponent() {

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

    void FlowBlockZELIF::simStartCurCycle() {
        assert(false);
    }

    void FlowBlockZELIF::simExitCurCycle() {
        assert(false);
    }

}
