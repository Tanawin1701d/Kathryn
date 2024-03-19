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
        /**we will not delete basic nodes due to node ownership transfering*/
        _basicNodes.clear();
    }

    void FlowBlockZIF::addElementInFlowBlock(Node* node) {
        assert(node != nullptr);
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockZIF::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZIF::addConFlowBlock(FlowBlockBase *fb) {
        assert(fb != nullptr);
        assert(fb->getFlowType() == ZELIF || fb->getFlowType() == ZELSE);
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
        buildSubHwComponent();
        assert(_subBlocks.empty());
        assert(!_basicNodes.empty());

        Operable* purifiedCurCond = purifyCondition(curCond);
        /**assign node condition to our basic Node */
        for (auto nd: _basicNodes){
            assert(nd != nullptr);
            assert(purifiedCurCond != nullptr);
            nd->addCondtion(purifiedCurCond, BITWISE_AND);
        }
        if (_conBlocks.empty())
            return;
        prevFalse = &(!(*purifiedCurCond));
        /**assign for each zelif block and zelse block*/
        bool elseDetected = false;

        for (auto zelifBlock: _conBlocks){
            assert(zelifBlock->getFlowType() == ZELIF ||
                   zelifBlock->getFlowType() == ZELSE);
            auto* castedZelifBlock = (FlowBlockZELIF*)zelifBlock;
            /** extract the sub element*/
            for (auto subBasicNode : castedZelifBlock->getBasicNode()){
                if (castedZelifBlock->getCurCond() == nullptr){
                    elseDetected = true;
                    subBasicNode->addCondtion(prevFalse,BITWISE_AND);
                }else{
                    assert(!elseDetected);
                    Operable* purifiedCond = purifyCondition(castedZelifBlock->getCurCond());
                    subBasicNode->addCondtion(&((*prevFalse) && (*purifiedCond)),
                                              BITWISE_AND);
                }

                _basicNodes.push_back(subBasicNode);
            }
            if (castedZelifBlock->getCurCond() != nullptr) {
                Operable* purifiedCond = purifyCondition(castedZelifBlock->getCurCond());
                prevFalse = &((*prevFalse) && !(*purifiedCond));
            }
        }

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