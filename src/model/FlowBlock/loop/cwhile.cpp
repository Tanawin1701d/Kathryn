//
// Created by tanawin on 2/2/2567.
//

#include "cwhile.h"


namespace kathryn{

    FlowBlockcWhile::FlowBlockcWhile(Operable& opr):
    FlowBlockWhileBase(opr, CWHILE){}

    FlowBlockcWhile::~FlowBlockcWhile(){
        delete loopNodeWrap;
        delete byPassExitNode;
        delete subBlockExitNode;
    }

    void FlowBlockcWhile::buildHwComponent() {
        assert(conBlocks.empty());
        assert(subBlocks.size() == 1);
        /** get node wrap */
        subBlockNodeWrap = subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);


        /** copy node and node wrap*/
        loopNodeWrap = new NodeWrap(*subBlockNodeWrap);


        /**assign to result node wrap*/
        /** node wrap no assign because we must sent to upper block*/
        resultNodeWrapper = new NodeWrap();
        exitNode          = new PseudoNode(1);
        byPassExitNode    = new PseudoNode(1);
        subBlockExitNode  = new PseudoNode(1);

        /**
         *
         * result node wrap
         *
         * */

        /** entrance result nodewrap*/
        resultNodeWrapper->transferEntNodeFrom(subBlockNodeWrap);
        resultNodeWrapper->addConditionToAllNode(_purifiedCondExpr, BITWISE_AND);
        /** in case it is not match at first time*/
        resultNodeWrapper->addEntraceNode(byPassExitNode);

        /**exit node wrap*/
        byPassExitNode->addCondtion(&(!*_purifiedCondExpr), BITWISE_AND);
        byPassExitNode->setDependStateJoinOp(BITWISE_AND);
        /** in case exit from sublock*/
        subBlockExitNode->addCondtion(&(!*_purifiedCondExpr), BITWISE_AND);
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
        if (subBlockNodeWrap->isThereForceExitNode()) {
            Operable* allowLoopCond = &((*_purifiedCondExpr) & (!*subBlockNodeWrap->getForceExitNode()->getExitOpr()));
            loopNodeWrap->addConditionToAllNode(allowLoopCond, BITWISE_AND);
        }else{
            loopNodeWrap->addConditionToAllNode(_purifiedCondExpr, BITWISE_AND);
        }
        loopNodeWrap->assignAllNode();
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

    void FlowBlockcWhile::simStartCurCycle() {
        if (isCurCycleSimulated()){
            return;
        }
        setSimStatus();
        bool isStateRunning = false;
        for (auto _sb: subBlocks){
            assert(_sb != nullptr);
            _sb->simStartCurCycle();
            isStateRunning |= _sb->isBlockOrNodeRunning();
        }
        for (auto _bsAsmNd: basicNodes){
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
        for (auto _sb: subBlocks){
            assert(_sb != nullptr);
            _sb->simExitCurCycle();
        }
        for (auto _bsAsmNd: basicNodes){
            assert(_bsAsmNd != nullptr);
            _bsAsmNd->simExitCurCycle();
        }
    }

}