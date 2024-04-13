//
// Created by tanawin on 2/2/2567.
//

#include "cwhile.h"


namespace kathryn{

    FlowBlockcWhile::FlowBlockcWhile(Operable& opr):
    FlowBlockWhileBase(opr, CWHILE){}

    FlowBlockcWhile::~FlowBlockcWhile(){
        delete upConditionNode;
        delete loopConditionNode;
        delete conditionNode;

        delete byPassExitNode;
        delete subBlockExitNode;
    }

    void FlowBlockcWhile::buildHwComponent() {

        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        /** get node wrap */
        subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);

        /**assign to result node wrap*/
        /** node wrap no assign because we must sent to upper block*/

        /**condition*/
        upConditionNode   = new PseudoNode(1);
        loopConditionNode = new PseudoNode(1);
        conditionNode     = new PseudoNode(1);
        /**exit*/
        byPassExitNode    = new PseudoNode(1);
        subBlockExitNode  = new PseudoNode(1);
        exitNode          = new PseudoNode(1);
        /**result*/
        resultNodeWrapper = new NodeWrap();

        /**
         * do condition node first
         * */
        upConditionNode->setDependStateJoinOp(BITWISE_AND);
        upConditionNode->addCondtion(_purifiedCondExpr, BITWISE_AND);
        upConditionNode->setInternalIdent("cWhileConNodeUpTrig" + std::to_string(getGlobalId()));

        loopConditionNode->setDependStateJoinOp(BITWISE_AND);
        loopConditionNode->addCondtion(_purifiedCondExpr, BITWISE_AND);
        loopConditionNode->addDependNode(subBlockNodeWrap->getExitNode());
        loopConditionNode->setInternalIdent("cWhileConNodeLoopBack" + std::to_string(getGlobalId()));
        loopConditionNode->assign();



        conditionNode->setDependStateJoinOp(BITWISE_OR);
        conditionNode->addDependNode(upConditionNode);
        conditionNode->addDependNode(loopConditionNode);
        if (_interruptNode[INTR_TYPE_START] != nullptr){
            conditionNode->addDependNode(_interruptNode[INTR_TYPE_START]);
        }
        conditionNode->setInternalIdent("cWhileConNode" + std::to_string(getGlobalId()));
        conditionNode->assign();

        /**
         * do work node wrap
         * **/
        subBlockNodeWrap->addDependNodeToAllNode(conditionNode);
        subBlockNodeWrap->assignAllNode();
        /**
         * exit node
         * */
        ////////// incase exit bypassing
        byPassExitNode->setDependStateJoinOp(BITWISE_AND);
        byPassExitNode->addCondtion(&(!*_purifiedCondExpr), BITWISE_AND);
        byPassExitNode->setInternalIdent("cWhilebypass" + std::to_string(getGlobalId()));
        ////////// incase exit from subblock
        subBlockExitNode->setDependStateJoinOp(BITWISE_AND);
        subBlockExitNode->addCondtion(&(!*_purifiedCondExpr), BITWISE_AND);
        subBlockExitNode->addDependNode(subBlockNodeWrap->getExitNode());
        subBlockExitNode->setInternalIdent("cWhilesubBlock" + std::to_string(getGlobalId()));
        subBlockExitNode->assign();
        /** pool exit node and put to result node wrap*/
        exitNode->setDependStateJoinOp(BITWISE_OR);
        exitNode->addDependNode(subBlockExitNode);
        exitNode->addDependNode(byPassExitNode);
        exitNode->setInternalIdent("cWhileExit" + std::to_string(getGlobalId()));
        if ( subBlockNodeWrap->isThereForceExitNode() ){
            exitNode->addDependNode(subBlockNodeWrap->getForceExitNode());
        }
        exitNode->assign();

        /**
         * result node wrap
         * */
        resultNodeWrapper->addEntraceNode(upConditionNode);
        resultNodeWrapper->addEntraceNode(byPassExitNode);
        resultNodeWrapper->addExitNode(exitNode);
        /**cycle andd force exit is set to -1*/

    }


    std::string FlowBlockcWhile::getMdDescribe() {
        std::string ret = FlowBlockBase::getMdIdentVal() + "\n";
        ret += "[cwhile]implicitFlowBlock " + implicitFlowBlock->getMdDescribe() + "\n";
        ret += "[cwhile]exitNode " + ((exitNode != nullptr) ? exitNode->getMdIdentVal() + "  " + exitNode->getMdDescribe() + "\n" : "\n");
        return ret;
    }

    void FlowBlockcWhile::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("exitNode " + ((exitNode != nullptr) ?
                                        exitNode->getMdIdentVal() + "  " +
                                        exitNode->getMdDescribe() :
                                        ""));
        mdLogVal->addVal("resultNodeWrap is "+ resultNodeWrapper->getMdIdentVal() +
                         " " + resultNodeWrapper->getMdDescribe());
        mdLogVal->addVal("implicitSubBlock");

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->addMdLog(subLog);

    }

    void FlowBlockcWhile::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isStateRunning = false;
        for (auto _sb: _subBlocks){
            assert(_sb != nullptr);
            _sb->simStartCurCycle();
            isStateRunning |= _sb->isBlockOrNodeRunning();
        }
        for (auto _bsAsmNd: _basicNodes){
            assert(_bsAsmNd != nullptr);
            _bsAsmNd->simStartCurCycle();
        }

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }
    }

    void FlowBlockcWhile::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        for (auto _sb: _subBlocks){
            assert(_sb != nullptr);
            _sb->simExitCurCycle();
        }
        for (auto _bsAsmNd: _basicNodes){
            assert(_bsAsmNd != nullptr);
            _bsAsmNd->simExitCurCycle();
        }
    }

}