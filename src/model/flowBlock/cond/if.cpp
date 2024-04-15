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
        delete condNode;
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
        /// allStatement.push_back(elifBlock->sumarizeBlock());
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
        /**summarize all block*/
        assert(_subBlocks.size() == 1);
        allStatement.push_back(_subBlocks[0]->sumarizeBlock());
        for (auto conFlowBlock: _conBlocks){
            allStatement.push_back(conFlowBlock->sumarizeBlock());
        }
        assert(!allCondes.empty());
        assert(allPurifiedCondes.size() == allCondes.size());
        assert(!allStatement.empty());


        if (getFlowType() == CIF){
            condNode = new PseudoNode(1, BITWISE_OR);
        }else if(getFlowType() == SIF){
            condNode = new StateNode();
            fillIntResetToNodeIfThere(condNode);
        }else{ assert(false);}
        condNode->setInternalIdent("ifExitNode" + std::to_string(getGlobalId()));
        exitNode = new PseudoNode(1, BITWISE_OR);
        exitNode->setInternalIdent("scifExitNode" + std::to_string(getGlobalId()));
        resultNodeWrap = new NodeWrap();

        /**add condition to state*/
        Operable* prevFalse = &(~(*allPurifiedCondes[0]));
        /** assign first first if*/
        allStatement[0]->addDependNodeToAllNode( condNode,allPurifiedCondes[0]);
        allStatement[0]->assignAllNode();
        exitNode->addDependNode(allStatement[0]->getExitNode(), nullptr);


        int statementId = 1;
        for (; statementId < allStatement.size(); statementId++){
            if (statementId < allPurifiedCondes.size()) {
                allStatement[statementId]->addDependNodeToAllNode(
                        condNode,
                        &((*allPurifiedCondes[statementId]) & (*prevFalse)));
                allStatement[statementId]->assignAllNode();
                exitNode->addDependNode(allStatement[statementId]->getExitNode(), nullptr);
                prevFalse = &((*prevFalse) & ~(*allPurifiedCondes[statementId]));
            }else{
                /** case else statement*/
                assert(statementId == (allPurifiedCondes.size())); /// check no ambiguous statement
                allStatement[statementId]->addDependNodeToAllNode(
                        condNode,
                        prevFalse);
                allStatement[statementId]->assignAllNode();
                exitNode->addDependNode(allStatement[statementId]->getExitNode(), nullptr);
            }
        }

        if (allStatement.size() == allPurifiedCondes.size()){
            /** there is no else node*/
            /** prev false is ready*/
            exitNode->addDependNode(condNode, prevFalse);
        }
        exitNode->assign();

        /**exit condition of node wrap*/
        resultNodeWrap->addEntraceNode(condNode);
        resultNodeWrap->addExitNode(exitNode);
        /**force exit condition*/
        genSumForceExitNode(allStatement);
        if (_areThereForceExit)
            resultNodeWrap->addForceExitNode(_forceExitNode);

        /**cycle determiner*/
        NodeWrapCycleDet deter;
        deter.addToDet(allStatement);
        if(allStatement.size() == allPurifiedCondes.size()){
            /** there is zero state node*/
            deter.addToDet(0);
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
        for (auto sb : _subBlocks){
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

        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();

        bool isStateRunning = false;
        /** simulate */
        for(auto sb: _subBlocks){
            sb->simStartCurCycle();
            isStateRunning |= sb->isBlockOrNodeRunning();
        }

        for(auto cb: _conBlocks){
            cb->simStartCurCycle();
            isStateRunning |= cb->isBlockOrNodeRunning();
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
        for(auto sb: _subBlocks){
            sb->simExitCurCycle();
        }
        for(auto cb: _conBlocks){
            cb->simExitCurCycle();
        }

        if (condNode != nullptr){
            condNode->simExitCurCycle();
        }

    }


}