//
// Created by tanawin on 2/2/2567.
//

#include "zif.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/cond/zelif.h"




namespace kathryn{


    FlowBlockZIF::FlowBlockZIF(Operable &cond):
            FlowBlockBase(ZIF,
                  {
                  { FLOW_ST_BASE_STACK,
                                 FLOW_ST_HEAD_COND_STACK},
                  FLOW_JO_EXT_FLOW,
                  true
                  }),
            curCond(&cond),
            prevFalse(nullptr)
    {
        ///assert(flowBlockType == ZIF);
        assert(curCond != nullptr);
    }

    FlowBlockZIF::~FlowBlockZIF() {
        basicNodes.clear();
        FlowBlockBase::~FlowBlockBase();
    }

    void FlowBlockZIF::addElementInFlowBlock(Node *node) {
        assert(node != nullptr);
        node->addCondtion(curCond, BITWISE_AND);
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockZIF::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZIF::addConFlowBlock(FlowBlockBase *fb) {
        assert(fb != nullptr);
        assert(fb->getFlowType() == ZELIF ||fb->getFlowType() == ZELSE);

        auto* elifFb = (FlowBlockZELIF*)fb;

        /***check condition*/
        if (prevFalse == nullptr){
            prevFalse = &(!*curCond);
        }else{
            prevFalse = &((!*lastElifCond) && (*prevFalse));
        }

        Operable* actualElif =  &((*prevFalse) && (*elifFb->getCurCond()));
        lastElifCond = elifFb->getCurCond();

        /**extract the conjunction block*/
        for (auto elifNode: getBasicNode()){
            assert(elifNode != nullptr);
            elifNode->addCondtion(actualElif, BITWISE_AND);
            basicNodes.push_back(elifNode);
        }

        /** call base function*/
        FlowBlockBase::addConFlowBlock(fb);
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