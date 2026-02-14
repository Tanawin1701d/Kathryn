//
// Created by tanawin on 14/2/26.
//

#include "loop.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockLoop::FlowBlockLoop(int loopCount):
    FlowBlockBase(CLOOP,
                  {
                      {FLOW_ST_BASE_STACK},
                      FLOW_JO_SUB_FLOW,
                      true
                  }),
    _loopCount(loopCount){
    }

    FlowBlockLoop::~FlowBlockLoop() {
        delete _resultNodeWrapper;
        delete _entNode;
        delete _loopNode;
        delete _exitNode;
    }


    void FlowBlockLoop::buildHwComponent(){
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        _subBlockNodeWrap = _subBlocks[0]->sumarizeBlock();
        assert(_subBlockNodeWrap != nullptr);


        //** initialize node*/
        if (getFlowType() == CWHILE){
            conditionNode = new PseudoNode(1, BITWISE_OR);
            conditionNode->setInternalIdent("cConNode" + std::to_string(getGlobalId()));
        }else{////// SWHILE
            conditionNode = new StateNode(getClockMode());
            conditionNode->setInternalIdent("sConNode" + std::to_string(getGlobalId()));
            fillIntResetToNodeIfThere(conditionNode);
            fillHoldToNodeIfThere(conditionNode);
        }

        _entNode = new PseudoNode(1, BITWISE_OR);
        _entNode->setInternalIdent("cEntNode" + std::to_string(getGlobalId()));
        addSysNode(_entNode);

        _loopNode = new PseudoNode(1, BITWISE_OR);
        _loopNode->setInternalIdent("cLoopNode" + std::to_string(getGlobalId()));
        addSysNode(_loopNode);

        _cntNode = new CounterReg(_loopCount);
        _exitNode = new PseudoNode(1, BITWISE_AND);

        exitNode = new PseudoNode()

        exitNode          = new PseudoNode(1, BITWISE_OR);
        addSysNode(exitNode);
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
        if(isThereIntStart()){
            conditionNode->addDependNode(intNodes[INT_START], nullptr);
        }
        /**do exit NOde Dep*/
        if (!_fallTrue) {
            exitNode->addDependNode(conditionNode, &(!(*_purifiedCondExpr)) );
        }
        if (subBlockNodeWrap->isThereForceExitNode()){
            exitNode->addDependNode(subBlockNodeWrap->getForceExitNode(), nullptr);
        }

        if (_fallTrue && (!subBlockNodeWrap->isThereForceExitNode())){
            ///////// incase there is no exit source we warning user that there is infinite loop
            /////////// TODO warning
            exitDummy = new DummyNode(&makeOprVal("exitDummy",1, 0));
            addSysNode(exitDummy);
            exitNode->addDependNode(exitDummy, nullptr);
        }

        exitNode->assign();


        resultNodeWrapper->addEntraceNode(conditionNode);
        resultNodeWrapper->addExitNode(exitNode);

    }



    void FlowBlockLoop::addElementInFlowBlock(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockLoop::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockLoop::sumarizeBlock() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockLoop::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockLoop::onDetachBlock() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->onDetachBlock();
        assert(isGetFlowBlockYet);
        ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockLoop::doPreFunction() {
        onAttachBlock();
    }
    void FlowBlockLoop::doPostFunction() {
        onDetachBlock();
    }

    void FlowBlockLoop::addMdLog(MdLogVal* mdLogVal){

        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        mdLogVal->addVal("entNode " + entNode->getMdIdentVal() + " " + entNode->getMdDescribe());
        mdLogVal->addVal("loopNode " + entNode->getMdIdentVal() + " " + loopNode->getMdDescribe());
        mdLogVal->addVal("exitNode " + exitNode->getMdIdentVal() + " " + exitNode->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrapper->getMdIdentVal() + " " + resultNodeWrapper->getMdDescribe());

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->addMdLog(subLog);

    }

}
