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

    FlowBlockZIF::~FlowBlockZIF() {}


    void FlowBlockZIF::addElementInFlowBlock(Node* node) {
        assert(node != nullptr);
        assert(node->getNodeType() == ASM_NODE);

        //////// cast meta data
        AsmNode* castedNode = (AsmNode*)node;
        FlowBlockBase::addAbandonNode(node);
        //////// try to add to
        tryAddOrCreateAsmMeta(castedNode, _assignMetas, purifiedCurCond);
    }

    void FlowBlockZIF::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZIF::addConFlowBlock(FlowBlockBase *fb){
        assert(!lastZelifDetected);
        assert(!prevFalses.empty());
        assert(fb != nullptr);
        assert(fb->getFlowType() == ZELIF || fb->getFlowType() == ZELSE);
        /** call base function*/
        FlowBlockBase::addConFlowBlock(fb);
        FlowBlockZELIF* castedZelif = (FlowBlockZELIF*)fb;

        Operable* prevFalse = *prevFalses.rbegin();

        /**
         *  try to merge with exist node because we need to minimize the internal generation
         */

        std::vector<ZifClassAsm*> subClassAsmMeta = castedZelif->getClassAssMetas();

        ///// it is not possible that slave will hit the same zif because the slave will handle it
        for (ZifClassAsm* slaveAsmMeta: subClassAsmMeta){
            bool found = false;
            for (ZifClassAsm* classAsm: _assignMetas){
                if (classAsm->isJoinable(slaveAsmMeta)){
                    classAsm->addZelifStage(slaveAsmMeta);
                    found = true;
                    break;
                }
            }
            if (!found){
                slaveAsmMeta->condition = addLogicWithOutput(prevFalse, slaveAsmMeta->condition, BITWISE_AND);
                _assignMetas.push_back(slaveAsmMeta);
            }
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

    void FlowBlockZIF::addIntSignal(INT_TYPE type, Operable* signal){
        mfAssert(!isThereIntStart(), "start interrupt can start in zblock");
        mfAssert(!isThereIntRst(), "start interrupt can reset in zblock");
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

    std::vector<AsmNode*> FlowBlockZIF::extract(){

        ///// generate the node
        std::vector<AsmNode*> result;
        for (ZifClassAsm* zifClassAsm: _assignMetas){
            result.push_back(zifClassAsm->createAsmNode());
        }
        return result;

    }
}