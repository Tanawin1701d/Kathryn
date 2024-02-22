//
// Created by tanawin on 6/12/2566.
//

#include "if.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockIf::FlowBlockIf(Operable& cond,
                             FLOW_BLOCK_TYPE flowBlockType):
    FlowBlockBase(flowBlockType,
       {
               {FLOW_ST_BASE_STACK,
                             FLOW_ST_HEAD_COND_STACK},
               FLOW_JO_SUB_FLOW,
               true
       }){
        assert(flowBlockType == CIF || flowBlockType == SIF);
        allCondes.push_back(&cond);
        allPurifiedCondes.push_back(purifyCondition(&cond));
    }

    FlowBlockIf::~FlowBlockIf(){
        delete psuedoElseNode;
        delete exitNode;
        delete resultNodeWrap;
    }

    void FlowBlockIf::addElementInFlowBlock(Node *node) {
        assert(false); /** this flow type will generate single par or seq block*/
    }

    void FlowBlockIf::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    void FlowBlockIf::addConFlowBlock(FlowBlockBase* conBlock){
        assert(conBlock != nullptr);
        assert(conBlock->getFlowType() == CSELIF ||
               conBlock->getFlowType() == CSELSE);
        /*** convert to elif block*/
        FlowBlockElif* elifBlock = (FlowBlockElif*)conBlock;
        /*** call base function*/
        FlowBlockBase::addConFlowBlock(elifBlock);
        /** push to if-else concern element*/
        allStatement.push_back(elifBlock->sumarizeBlock());
        if (elifBlock->getCondition() != nullptr) {
            allCondes.push_back(elifBlock->getCondition());
            allPurifiedCondes.push_back(
                    purifyCondition(elifBlock->getCondition())
            );
        }

        assert(conBlock->getSubBlocks()[0] != nullptr);
    }

    NodeWrap* FlowBlockIf::sumarizeBlock() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockIf::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockIf::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        ////// we will hold this end block will handle it
        setLazyDelete();

        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockIf::buildHwComponent() {

        assert(!allCondes.empty());
        assert(allPurifiedCondes.size() == allCondes.size());
        /**add execution block in if block to consider vector*/
        allStatement.insert(allStatement.begin(), subBlocks[0]->sumarizeBlock());
        assert(!allStatement.empty());

        /**add condition to state*/
        Operable* prevFalse = &(~(*allPurifiedCondes[0]));
        /** assign first first if*/
        allStatement[0]->addConditionToAllNode(allPurifiedCondes[0], BITWISE_AND);
        int statementId = 1;
        for (; statementId < allStatement.size(); statementId++){
            if (statementId < allPurifiedCondes.size()) {
                allStatement[statementId]->addConditionToAllNode(
                        &((*allPurifiedCondes[statementId]) & (*prevFalse)),
                        BITWISE_AND);
                prevFalse = &((*prevFalse) & ~(*allPurifiedCondes[statementId]));
            }else{
                /** case else statement*/
                assert(statementId == (allPurifiedCondes.size())); /// check no ambiguous statement
                allStatement[statementId]->addConditionToAllNode(
                        prevFalse,
                        BITWISE_AND);
            }
        }

        /** prepare return node wrap*/
        resultNodeWrap = new NodeWrap();


        /*** head of block conditioning*/
        if (getFlowType() == CIF){
            for (auto nw : allStatement){
                resultNodeWrap->transferEntNodeFrom(nw);
            }
            ///// build proxy node to prevent state lost
            if ( allCondes.size() == allStatement.size() ) {
                psuedoElseNode = new PseudoNode(1);
                psuedoElseNode->setDependStateJoinOp(BITWISE_AND);
                psuedoElseNode->addCondtion(prevFalse, BITWISE_AND);
                resultNodeWrap->addEntraceNode(psuedoElseNode);
            }
        }else if(getFlowType() == SIF){
            condNode = new StateNode();
            condNode->setDependStateJoinOp(BITWISE_AND);
            resultNodeWrap->addEntraceNode(condNode);
            for (auto nw: allStatement){
                nw->addDependNodeToAllNode(condNode);
                nw->assignAllNode();
            }
            if ( allCondes.size() == allStatement.size() ) {
                psuedoElseNode = new PseudoNode(1);
                psuedoElseNode->addCondtion(prevFalse, BITWISE_AND);
                psuedoElseNode->addDependNode(condNode);
                psuedoElseNode->setDependStateJoinOp(BITWISE_AND);
                psuedoElseNode->assign();
            }
        }



        /**exit condition of node wrap*/
        exitNode = new PseudoNode(1);
        exitNode->setDependStateJoinOp(BITWISE_OR);
        for (auto nw : allStatement){
            exitNode->addDependNode(nw->getExitNode());
        }
        if (psuedoElseNode != nullptr)
            exitNode->addDependNode(psuedoElseNode);
        exitNode->assign();
        resultNodeWrap->addExitNode(exitNode);

        /**force exit condition*/
        genSumForceExitNode(allStatement);
        if (areThereForceExit)
            resultNodeWrap->addForceExitNode(forceExitNode);

        /**cycle determiner*/
        NodeWrapCycleDet deter;
        deter.addToDet(allStatement);
        if (psuedoElseNode != nullptr){
            deter.addToDet(psuedoElseNode);
        }
        /**cycle determiner for node wrap*/
        int cycleUsed = deter.getSameCycleHorizon();
        if (cycleUsed == IN_CONSIST_CYCLE_USED){
            resultNodeWrap->setCycleUsed(IN_CONSIST_CYCLE_USED);
        }else if (getFlowType() == CIF){
            resultNodeWrap->setCycleUsed(cycleUsed);
        }else if (getFlowType() == SIF){
            resultNodeWrap->setCycleUsed(cycleUsed+1);
        }else{
            assert(false);
        }


    }


    std::string FlowBlockIf::getMdDescribe() {
        std::string ret;
        ret += "[ " + FlowBlockBase::getMdIdentVal() +" ]\n";
        ret += "exitNode is " + ((exitNode != nullptr) ? exitNode->getMdIdentVal()+ "  " + exitNode->getMdDescribe(): "") + "\n";
        ret += "[implicitFlowBlock]" + implicitFlowBlock->getMdDescribe() + "\n";
        return ret;
    }

    void FlowBlockIf::addMdLog(MdLogVal *mdLogVal) {
        assert(allCondes.size() == allPurifiedCondes.size());
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() +" ]");
        int cnt = 0;
        if (resultNodeWrap->isThereForceExitNode()){
            mdLogVal->addVal("forceExit is " + resultNodeWrap->getForceExitNode()->getMdIdentVal() +
                             "  " +
                             resultNodeWrap->getForceExitNode()->getMdDescribe());
        }
        for (auto sb : subBlocks){
            std::string subBlockHeaderDebug = "----> subblock " + std::to_string(cnt) + " condition ";
            if (cnt < allCondes.size()) {
                subBlockHeaderDebug += allCondes[cnt]->castToIdent()->getIdentDebugValue();
            }
            mdLogVal->addVal(subBlockHeaderDebug);

            sb->addMdLog(mdLogVal->makeNewSubVal());
            cnt++;
        }
    }

    void FlowBlockIf::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockIf::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockIf::simStartCurCycle() {

        if (isCurCycleSimulated()){
            return;
        }
        setSimStatus();

        bool isStateRunning = false;
        /** simulate */
        for(auto sb: subBlocks){
            sb->simStartCurCycle();
            isStateRunning |= sb->isBlockOrNodeRunning();
        }

        if (condNode != nullptr){
            condNode->simStartCurCycle();
            isStateRunning |= condNode->isBlockOrNodeRunning();
        }

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }


    }

    void FlowBlockIf::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        for(auto sb: subBlocks){
            sb->simExitCurCycle();
        }

        if (condNode != nullptr){
            condNode->simExitCurCycle();
        }

    }


}