//
// Created by tanawin on 2/2/2567.
//

#include "zif.h"
#include "model/controller/controller.h"




namespace kathryn{


    FlowBlockZIF::FlowBlockZIF(Operable &cond):
            FlowBlockBase(ZIF,
                  {
                  { FLOW_ST_BASE_STACK,
                                 FLOW_ST_HEAD_COND_STACK},
                  FLOW_JO_EXT_FLOW,
                  true
                  }),
            purifiedCurCond(&(*purifyCondition(&cond)))
    {
        ///assert(flowBlockType == ZIF);
        prevFalses.push_back(&(!(*purifiedCurCond)));
        assert(purifiedCurCond != nullptr);
    }

    FlowBlockZIF::~FlowBlockZIF() {
        /**we will not delete basic nodes due to node ownership transfering*/
        _basicNodes.clear();
    }

    void FlowBlockZIF::addElementInFlowBlock(Node* node) {
        assert(node != nullptr);
        FlowBlockBase::addElementInFlowBlock(node);
        node->addCondtion(purifiedCurCond, BITWISE_AND);
    }

    void FlowBlockZIF::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZIF::addConFlowBlock(FlowBlockBase *fb) {
        assert(!lastZelifDetected);
        assert(!prevFalses.empty());
        assert(fb != nullptr);
        assert(fb->getFlowType() == ZELIF || fb->getFlowType() == ZELSE);
        /** call base function*/
        FlowBlockBase::addConFlowBlock(fb);
        FlowBlockZELIF* castedZelif = (FlowBlockZELIF*)fb;

        Operable* prevFalse = *prevFalses.rbegin();
        /**get next activate cond*/
        /** assign activate cond and extract node*/
        for (auto insideNode: fb->getBasicNode()){
            assert(insideNode != nullptr);
            insideNode->addCondtion(*prevFalses.rbegin(), BITWISE_AND);
            FlowBlockBase::addElementInFlowBlock(insideNode);
        }
        /** generate next prev false*/
        if (castedZelif->getPurifiedCurCond() == nullptr){
            lastZelifDetected = true;
        }else{
            prevFalses.push_back(
                    &((*prevFalse) & (!(*castedZelif->getPurifiedCurCond())))
            );
        }

    }

    NodeWrap *FlowBlockZIF::sumarizeBlock() {
        assert(false);
    }

    void FlowBlockZIF::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZIF::onDetachBlock() {
        setLazyDelete();
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZIF::buildHwComponent() {
        assert(false);
    }

    std::string FlowBlockZIF::getMdDescribe() {
        assert(false);
    }

    void FlowBlockZIF::addMdLog(MdLogVal *mdLogVal) {
        assert(false);
    }

    void FlowBlockZIF::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockZIF::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockZIF::simStartCurCycle(){
        assert(false);
    }

    void FlowBlockZIF::simExitCurCycle(){
        assert(false);
    }

}