//
// Created by tanawin on 6/12/2566.
//

#include "if.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/cond/elif.h"


namespace kathryn{


    FlowBlockIf::FlowBlockIf(Operable& cond):FlowBlockBase(IF){
        allCondes.push_back(&cond);
    }

    void FlowBlockIf::addElementInFlowBlock(Node *node) {
        assert(true); /** this flow type will generate single par or seq block*/
    }

    void FlowBlockIf::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockIf::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockIf::onAttachBlock() {
        ctrl->on_attach_flowBlock_if(this);
        auto sb = genImplicitSubBlk(PARALLEL);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockIf::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        ////// we will hold this end block will handle it
        setLazyDelete();
    }

    void FlowBlockIf::buildHwComponent() {
        assert(!allCondes.empty());
        allStatement.insert(allStatement.begin(), subBlocks[0]->sumarizeBlock());

        assert(!allStatement.empty());

        /**add condition to state*/
        Operable* prevFalse = &(!(*allCondes[0]));
        /** assign first first if*/
        allStatement[0]->addConditionToAllNode(allCondes[0], BITWISE_AND);
        int statementId = 1;
        for (; statementId < allStatement.size(); statementId++){
            if (statementId < allCondes.size()) {
                allStatement[statementId]->addConditionToAllNode(
                        &((*allCondes[statementId]) & (*prevFalse)),
                        BITWISE_AND);
                prevFalse = &((*prevFalse) & !(*allCondes[statementId]));
            }else{
                /** case else statement*/
                assert(statementId == (allCondes.size())); /// check no ambiguous statement
                allStatement[statementId]->addConditionToAllNode(
                        prevFalse,
                        BITWISE_AND);
            }
        }

        /** prepare return node wrap*/
        resultNodeWrapper = new NodeWrap();
        for (auto nw : allStatement){
            resultNodeWrapper->transferNodeFrom(nw);
        }
        ///// build proxy node to prevent state lost
        Node* psuedoNode = nullptr;
        if ( allCondes.size() == allStatement.size() ) {
            psuedoNode = new Node();
            psuedoNode->addCondtion(prevFalse, BITWISE_AND);
            resultNodeWrapper->addEntraceNode(psuedoNode);
        }

        /**exit condition of node wrap*/
        Operable* exitCond = allStatement[0]->exitOpr;
        for (int i = 1; i < allStatement.size(); i++){
            exitCond = &((*exitCond) | (*allStatement[i]->exitOpr));
        }
        if (psuedoNode != nullptr){
            exitCond = &((*exitCond) | (*psuedoNode->psudoAssignMeta));
        }
        resultNodeWrapper->addExitOpr(exitCond);
        /** determine node wrap cycle */
        NodeWrapCycleDet deter;
        deter.addToDet(allStatement);
        if (psuedoNode != nullptr){
            deter.addToDet(psuedoNode);
        }
        resultNodeWrapper->setCycleUsed(deter.getSameCycleHorizon());

    }

    void FlowBlockIf::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockIf::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockIf::addElifNodeWrap(FlowBlockElif* fb) {
        assert(fb != nullptr);
        allStatement.push_back(fb->sumarizeBlock());
        if (fb->getCondition() != nullptr)
            allCondes.push_back(fb->getCondition());
        assert(fb->getSubBlocks()[0] != nullptr);
        subBlocks.push_back(fb->getSubBlocks()[0]);
        delete fb;
    }


}