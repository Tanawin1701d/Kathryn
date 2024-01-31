//
// Created by tanawin on 6/12/2566.
//

#include "if.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockIf::FlowBlockIf(Operable& cond):FlowBlockBase(IF){
        allCondes.push_back(&cond);
    }

    FlowBlockIf::~FlowBlockIf(){
        delete psuedoElseNode;
        delete exitNode;
        delete resultNodeWrap;
        FlowBlockBase::~FlowBlockBase();
    }

    void FlowBlockIf::addElementInFlowBlock(Node *node) {
        assert(false); /** this flow type will generate single par or seq block*/
    }

    void FlowBlockIf::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockIf::sumarizeBlock() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockIf::onAttachBlock() {
        ctrl->on_attach_flowBlock_if(this);
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
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
        /**add execution block in if block to consider vector*/
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
        resultNodeWrap = new NodeWrap();
        for (auto nw : allStatement){
            resultNodeWrap->transferEntNodeFrom(nw);
        }
        ///// build proxy node to prevent state lost
        if ( allCondes.size() == allStatement.size() ) {
            psuedoElseNode = new PseudoNode(1);
            psuedoElseNode->addCondtion(prevFalse, BITWISE_AND);
            resultNodeWrap->addEntraceNode(psuedoElseNode);
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
        resultNodeWrap->setCycleUsed(deter.getSameCycleHorizon());

    }


    std::string FlowBlockIf::getMdDescribe() {
        std::string ret;
        ret += "[ " + FlowBlockBase::getMdIdentVal() +" ]\n";
        ret += "exitNode is " + ((exitNode != nullptr) ? exitNode->getMdIdentVal()+ "  " + exitNode->getMdDescribe(): "") + "\n";
        ret += "[implicitFlowBlock]" + implicitFlowBlock->getMdDescribe() + "\n";
        return ret;
    }

    void FlowBlockIf::addMdLog(MdLogVal *mdLogVal) {
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

    void FlowBlockIf::addElifNodeWrap(FlowBlockElif* fb) {
        assert(fb != nullptr);
        allStatement.push_back(fb->sumarizeBlock());
        if (fb->getCondition() != nullptr)
            allCondes.push_back(fb->getCondition());
        assert(fb->getSubBlocks()[0] != nullptr);
        subBlocks.push_back(fb);
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
        if (psuedoElseNode != nullptr){
            psuedoElseNode->simStartCurCycle();
            isStateRunning |= psuedoElseNode->isBlockOrNodeRunning();
        }

        if (exitNode != nullptr){
            exitNode->simStartCurCycle();
            isStateRunning |= exitNode->isBlockOrNodeRunning();
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
        if (psuedoElseNode != nullptr){
            psuedoElseNode->simExitCurCycle();
        }

        if (exitNode != nullptr){
            exitNode->simExitCurCycle();
        }

    }


}