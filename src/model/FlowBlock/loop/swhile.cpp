//
// Created by tanawin on 2/2/2567.
//

#include "swhile.h"


namespace kathryn{


    FlowBlockSWhile::FlowBlockSWhile(Operable &opr) :
        FlowBlockWhileBase(opr, SWHILE) {}

    FlowBlockSWhile::~FlowBlockSWhile(){
        delete conNode;
        delete backToConNode;
        delete normalExit;
    }

    void FlowBlockSWhile::buildHwComponent() {
        buildSubHwComponent();

        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);

        resultNodeWrapper = new NodeWrap();
        conNode           = new StateNode();
        backToConNode     = new PseudoNode(1);

        normalExit        = new PseudoNode(1);
        exitNode          = new PseudoNode(1);

        /***
         * recurrent node
         *
         * */

        /***node that intend to reLoop*/
        backToConNode->addDependNode(subBlockNodeWrap->getExitNode());
        if (subBlockNodeWrap->getForceExitNode() != nullptr)
            backToConNode->addCondtion(
                    &!*subBlockNodeWrap->getForceExitNode()->getExitOpr(),
                    BITWISE_AND);
        backToConNode->setDependStateJoinOp(BITWISE_AND);
        backToConNode->assign();

        /***con node  no need to assign due to it upper block assignment**/
        conNode->addDependNode(backToConNode);
        conNode->setDependStateJoinOp(BITWISE_OR);
        /***
         *
         * exit node
         *
         * */
        /**node that intend to exit*/
        normalExit->addDependNode(conNode);
        normalExit->addCondtion(&(!*_purifiedCondExpr), BITWISE_AND);
        normalExit->setDependStateJoinOp(BITWISE_AND);
        normalExit->assign();


        exitNode->addDependNode(normalExit);
        if (subBlockNodeWrap->isThereForceExitNode())
            exitNode->addDependNode(subBlockNodeWrap->getForceExitNode());
        exitNode->setDependStateJoinOp(BITWISE_OR);
        exitNode->assign();


        /***
         *
         * result node wrap and subblock connectivity
         *
         */
         subBlockNodeWrap->addDependNodeToAllNode(conNode);
         subBlockNodeWrap->addConditionToAllNode(_purifiedCondExpr, BITWISE_AND);
         subBlockNodeWrap->assignAllNode();

         resultNodeWrapper->addEntraceNode(conNode);
         resultNodeWrapper->addExitNode(exitNode);

    }

    std::string FlowBlockSWhile::getMdDescribe(){
        return "[SWHILE] getMdDescribe is not implemented\n";
    }

    void FlowBlockSWhile::addMdLog(MdLogVal* mdLogVal){

        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("conNode " + conNode->getMdIdentVal() + " " + conNode->getMdDescribe());
        mdLogVal->addVal("backToConNode " + backToConNode->getMdIdentVal() + " " + backToConNode->getMdDescribe());
        mdLogVal->addVal("normalExit " + normalExit->getMdIdentVal() + " " + normalExit->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
                             resultNodeWrapper->getMdIdentVal() + " " + resultNodeWrapper->getMdDescribe());

        mdLogVal->addVal("implicitSubBlock");

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->addMdLog(subLog);

    }

    void FlowBlockSWhile::simStartCurCycle() {

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
        assert(conNode != nullptr);
        conNode->simStartCurCycle();
        isStateRunning |= conNode->isBlockOrNodeRunning();

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }


    }

    void FlowBlockSWhile::simExitCurCycle() {
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
        assert(conNode != nullptr);
        conNode->simExitCurCycle();

    }


}