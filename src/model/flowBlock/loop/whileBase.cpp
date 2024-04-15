//
// Created by tanawin on 6/12/2566.
//

#include "whileBase.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockWhile::FlowBlockWhile(Operable& condExpr,
                                   FLOW_BLOCK_TYPE fbt):
    _condExpr(&condExpr),
    _purifiedCondExpr(purifyCondition(&condExpr)),
    FlowBlockBase(fbt,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }) {
        assert(_purifiedCondExpr != nullptr);
    }

    FlowBlockWhile::FlowBlockWhile(bool fallTrue,
                                   FLOW_BLOCK_TYPE fbt):
            _fallTrue(fallTrue),
            FlowBlockBase(fbt,{
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }) {
        assert(fallTrue);
        assert(_purifiedCondExpr == nullptr);
    }



    FlowBlockWhile::~FlowBlockWhile() {
        delete resultNodeWrapper;
        delete conditionNode;
        delete exitNode;
    }


    void FlowBlockWhile::buildHwComponent(){
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(subBlockNodeWrap != nullptr);


        //** initialize node*/
        if (getFlowType() == CWHILE){
            conditionNode = new PseudoNode(1, BITWISE_OR);
        }else{////// SWHILE
            conditionNode = new StateNode();
            fillIntResetToNodeIfThere(conditionNode);
        }
        exitNode          = new PseudoNode(1, BITWISE_OR);
        resultNodeWrapper = new NodeWrap();
        ////////////////////////////////////////////////////////////////////

        /** do sub block dep init*/
        subBlockNodeWrap->addDependNodeToAllNode(conditionNode, _purifiedCondExpr);
        subBlockNodeWrap->assignAllNode();

        /**do condition node Dep*/
            //// codition trigger from outside willbe trigger in upper node
        conditionNode->addDependNode(subBlockNodeWrap->getExitNode(),
                                     subBlockNodeWrap->isThereForceExitNode()?
                                        ( &(~(*subBlockNodeWrap->getForceExitNode()->getExitOpr())) ):
                                        nullptr
                                     );
        /**do exit NOde Dep*/
        if (!_fallTrue) {
            exitNode->addDependNode(conditionNode, &(!(*_purifiedCondExpr)) );
        }
        if (subBlockNodeWrap->isThereForceExitNode()){
            exitNode->addDependNode(subBlockNodeWrap->getForceExitNode(), nullptr);
        }
        exitNode->assign();


        resultNodeWrapper->addEntraceNode(conditionNode);
        resultNodeWrapper->addExitNode(exitNode);

    }



    void FlowBlockWhile::addElementInFlowBlock(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockWhile::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockWhile::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockWhile::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockWhile::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockWhile::doPreFunction() {
        onAttachBlock();
    }
    void FlowBlockWhile::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockWhile::addMdLog(MdLogVal* mdLogVal){

        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("conNode " + conditionNode->getMdIdentVal() + " " + conditionNode->getMdDescribe());
        mdLogVal->addVal("exitNode " + exitNode->getMdIdentVal() + " " + exitNode->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrapper->getMdIdentVal() + " " + resultNodeWrapper->getMdDescribe());

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->addMdLog(subLog);

    }

    void FlowBlockWhile::simStartCurCycle() {

        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isStateRunning = false;

        /** simulate block */
        for (auto _sb: _subBlocks){
            assert(_sb != nullptr);
            _sb->simStartCurCycle();
            isStateRunning |= _sb->isBlockOrNodeRunning();
        }
        for (auto _bsAsmNd: _basicNodes){
            assert(_bsAsmNd != nullptr);
            _bsAsmNd->simStartCurCycle();
        }

        /** conNode*/
        assert(conditionNode != nullptr);
        conditionNode->simStartCurCycle();
        isStateRunning |= conditionNode->isBlockOrNodeRunning();

        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }
    }

    void FlowBlockWhile::simExitCurCycle() {
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
        assert(conditionNode != nullptr);
        conditionNode->simExitCurCycle();

    }

}