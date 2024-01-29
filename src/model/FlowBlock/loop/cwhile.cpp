//
// Created by tanawin on 6/12/2566.
//

#include "cwhile.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockCwhile::FlowBlockCwhile(Operable& condExpr): _condExpr(&condExpr),
                                                          FlowBlockBase(WHILE) {}

    FlowBlockCwhile::~FlowBlockCwhile() {
        delete resultNodeWrapper;
        loopNodeWrap->deleteNodesInWrap();
        delete loopNodeWrap;
        delete exitNode;
        delete byPassExitNode;
        delete subBlockExitNode;
        FlowBlockBase::~FlowBlockBase();
    }

    void FlowBlockCwhile::addElementInFlowBlock(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockCwhile::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockCwhile::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockCwhile::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockCwhile::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockCwhile::buildHwComponent() {
        assert(subBlocks.size() == 1);
        /** get node wrap */
        subBlockNodeWrap = subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);


        /** copy node and node wrap*/
        loopNodeWrap = new NodeWrap(*subBlockNodeWrap);


        /**assign to result node wrap*/
        /** node wrap no assign because we must sent to upper block*/
        resultNodeWrapper = new NodeWrap();
        exitNode          = new PseudoNode();
        byPassExitNode    = new PseudoNode();
        subBlockExitNode  = new PseudoNode();

        /**
         *
         * result node wrap
         *
         * */

        /** entrance result nodewrap*/
        resultNodeWrapper->transferEntNodeFrom(subBlockNodeWrap);
        resultNodeWrapper->addConditionToAllNode(_condExpr, BITWISE_AND);
        /** in case it is not match at first time*/
        resultNodeWrapper->addEntraceNode(byPassExitNode);

        /**exit node wrap*/
        byPassExitNode->addCondtion(&(!*_condExpr), BITWISE_AND);
        /** in case exit from sublock*/
        subBlockExitNode->addCondtion(&(!*_condExpr), BITWISE_AND);
        subBlockExitNode->addDependNode(subBlockNodeWrap->getExitNode());
        subBlockExitNode->setDependStateJoinOp(BITWISE_AND);
        subBlockExitNode->assign();
        /** pool exit node and put to result node wrap*/
        exitNode->addDependNode(subBlockExitNode);
        exitNode->addDependNode(byPassExitNode);
        if ( subBlockNodeWrap->isThereForceExitNode() ){
            exitNode->addDependNode(subBlockNodeWrap->getForceExitNode());
        }
        exitNode->setDependStateJoinOp(BITWISE_OR);
        exitNode->assign();
        resultNodeWrapper->addExitNode(exitNode);


        /**
         * loop back condition to make block repeat
         * */

        /**assign for loop back assignment*/
        loopNodeWrap->addDependNodeToAllNode(subBlockNodeWrap->getExitNode());
        loopNodeWrap->setAllDependNodeCond(BITWISE_AND);
        if (subBlockNodeWrap->isThereForceExitNode()) {
            Operable *allowLoopCond = &((*_condExpr) & (!*subBlockNodeWrap->getForceExitNode()->getExitOpr()));
            loopNodeWrap->addConditionToAllNode(allowLoopCond, BITWISE_AND);
        }
        else {
            loopNodeWrap->addConditionToAllNode(_condExpr, BITWISE_AND);
        }
        loopNodeWrap->assignAllNode();
    }


    std::string FlowBlockCwhile::getMdDescribe() {
        std::string ret = FlowBlockBase::getMdIdentVal() + "\n";
        ret += "[cwhile]implicitFlowBlock " + implicitFlowBlock->getMdDescribe() + "\n";
        ret += "[cwhile]exitNode " + ((exitNode != nullptr) ? exitNode->getMdIdentVal() + "  " + exitNode->getMdDescribe() + "\n" : "\n");
        return ret;
    }

    void FlowBlockCwhile::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("exitNode " + ((exitNode != nullptr) ?
                                             exitNode->getMdIdentVal() + "  " +
                                             exitNode->getMdDescribe() :
                                             ""));
        mdLogVal->addVal("resultNodeWrap is "+ resultNodeWrapper->getMdIdentVal() +
        " " + resultNodeWrapper->getMdDescribe());
        mdLogVal->addVal("loopNodeWrap is "+ loopNodeWrap->getMdIdentVal() +
        " " + loopNodeWrap->getMdDescribe());
        mdLogVal->addVal("--------- explain loop Node wrap --------------");
        for (auto etNode : loopNodeWrap->entranceNodes){
            mdLogVal->addVal("       " + etNode->getMdIdentVal() + "   " + etNode->getMdDescribe());
        }
        mdLogVal->addVal("---------end explain loop node wrap --------------");

        mdLogVal->addVal("implicitSubBlock");

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->addMdLog(subLog);

    }

    void FlowBlockCwhile::doPreFunction() {
        onAttachBlock();
    }
    void FlowBlockCwhile::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockCwhile::simStartCurCycle() {
        if (isCurCycleSimulated()){
            return;
        }
        setSimStatus();

        bool isStateRunning = false;
        if (implicitFlowBlock != nullptr){
            implicitFlowBlock->simStartCurCycle();
            isStateRunning |= implicitFlowBlock->isBlockOrNodeRunning();
        }
        if (exitNode != nullptr){
            exitNode->simStartCurCycle();
            isStateRunning |= exitNode->isBlockOrNodeRunning();
        }
        if (byPassExitNode != nullptr){
            byPassExitNode->simStartCurCycle();
            isStateRunning |= byPassExitNode->isBlockOrNodeRunning();
        }
        if (subBlockExitNode != nullptr){
            subBlockExitNode->simStartCurCycle();
            isStateRunning |= subBlockExitNode->isBlockOrNodeRunning();
        }

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();

        }

    }

    void FlowBlockCwhile::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        if (implicitFlowBlock != nullptr){
            implicitFlowBlock->simExitCurCycle();
        }
        if (exitNode != nullptr){
            exitNode->simExitCurCycle();
        }
        if (byPassExitNode != nullptr){
            byPassExitNode->simExitCurCycle();
        }
        if (subBlockExitNode != nullptr){
            subBlockExitNode->simExitCurCycle();
        }

    }


}