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
    _loopCount(loopCount),
    _loopId(new expression(calBitUsedInCounter(_loopCount))){


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


        _entNode = new PseudoNode(1, BITWISE_OR);
        _entNode->setInternalIdent("cEntNode" + std::toString(getGlobalId()));
        addSysNode(_entNode);

        _loopNode = new PseudoNode(1, BITWISE_OR);
        _loopNode->setInternalIdent("cLoopNode" + std::toString(getGlobalId()));
        addSysNode(_loopNode);

        _cntNode = new CounterNode(_loopCount, getClockMode());
        _cntNode->setInternalIdent("countNode" + std::toString(getGlobalId()));
        addSysNode(_cntNode);

        _exitNode = new PseudoNode(1, BITWISE_AND);
        _exitNode->setInternalIdent("cExitNode" + std::toString(getGlobalId()));
        addSysNode(_exitNode);


        ////// handle start signal
        if(isThereIntStart()){
            _entNode->addDependNode(intNodes[INT_START], nullptr);
        }
        ////// no need to reset or hold the system


        ////// loop node
        _loopNode->addDependNode(_entNode, nullptr);
        _loopNode->addDependNode(_subBlockNodeWrap->getExitNode(),
                                 &(~(*_cntNode->getExitOprPtr())));
        _loopNode->assign();

        ////// counter Node
        _cntNode->addDependNode(_entNode, nullptr);
        _cntNode->makeIncCounterEvent(_subBlockNodeWrap->getExitNode());
        _cntNode->assign();

        ////// exit node
        _exitNode->addDependNode(_subBlockNodeWrap->getExitNode(),
                                 _cntNode->getExitOprPtr());
        _exitNode->assign();

        /////// sub block trigger
        _subBlockNodeWrap->addDependNodeToAllNode(_loopNode, nullptr);
        _subBlockNodeWrap->assignAllNode();


        _resultNodeWrapper = new NodeWrap();
        _resultNodeWrapper->addEntraceNode(_entNode);
        _resultNodeWrapper->addExitNode(_exitNode);
        if (_subBlockNodeWrap->getCycleUsed() != IN_CONSIST_CYCLE_USED){
            _resultNodeWrapper->setCycleUsed(_subBlockNodeWrap->getCycleUsed() * _loopCount);
        }
        if (_subBlockNodeWrap->isThereForceExitNode()){
            _resultNodeWrapper->addForceExitNode(_subBlockNodeWrap->getForceExitNodePtr());
        }

        (*_loopId) = (*_cntNode->getCounterPtr());

    }



    void FlowBlockLoop::addElementInFlowBlock(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockLoop::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!_isGetFlowBlockYet);
        _isGetFlowBlockYet = true;
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockLoop::sumarizeBlock() {
        assert(_resultNodeWrapper != nullptr);
        return _resultNodeWrapper;
    }

    void FlowBlockLoop::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = genImplicitSubBlk(PARALLEL_NO_SYN);
        _implicitFlowBlock = sb;
        sb->onAttachBlock();
    }

    void FlowBlockLoop::onDetachBlock() {
        assert(_implicitFlowBlock != nullptr);
        _implicitFlowBlock->onDetachBlock();
        assert(_isGetFlowBlockYet);
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
        mdLogVal->addVal("entNode " + _entNode->getMdIdentVal() + " " + _entNode->getMdDescribe());
        mdLogVal->addVal("loopNode " + _entNode->getMdIdentVal() + " " + _loopNode->getMdDescribe());
        mdLogVal->addVal("cntNode " + _cntNode->getMdIdentVal() + " " + _cntNode->getMdDescribe());
        mdLogVal->addVal("exitNode " + _exitNode->getMdIdentVal() + " " + _exitNode->getMdDescribe());
        mdLogVal->addVal("resultNodeWrap is" +
            _resultNodeWrapper->getMdIdentVal() + " " + _resultNodeWrapper->getMdDescribe());

        auto subLog = mdLogVal->makeNewSubVal();
        _implicitFlowBlock->addMdLog(subLog);
    }

}
