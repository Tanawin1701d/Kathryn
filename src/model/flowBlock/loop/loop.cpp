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
        _entNode->set_internal_ident("cEntNode" + std::to_string(get_global_id()));
        addSysNode(_entNode);

        _loopNode = new PseudoNode(1, BITWISE_OR);
        _loopNode->set_internal_ident("cLoopNode" + std::to_string(get_global_id()));
        addSysNode(_loopNode);

        _cntNode = new CounterNode(_loopCount, getClockMode());
        _cntNode->set_internal_ident("countNode" + std::to_string(get_global_id()));
        addSysNode(_cntNode);

        _exitNode = new PseudoNode(1, BITWISE_AND);
        _exitNode->set_internal_ident("cExitNode" + std::to_string(get_global_id()));
        addSysNode(_exitNode);


        ////// handle start signal
        if(isThereIntStart()){
            _entNode->add_depend_node(intNodes[INT_START], nullptr);
        }
        ////// no need to reset or hold the system


        ////// loop node
        _loopNode->add_depend_node(_entNode, nullptr);
        _loopNode->add_depend_node(_subBlockNodeWrap->getExitNode(),
                                 &(~(*_cntNode->get_exit_opr_ptr())));
        _loopNode->assign();

        ////// counter Node
        _cntNode->add_depend_node(_entNode, nullptr);
        _cntNode->makeIncCounterEvent(_subBlockNodeWrap->getExitNode());
        _cntNode->assign();

        ////// exit node
        _exitNode->add_depend_node(_subBlockNodeWrap->getExitNode(),
                                 _cntNode->get_exit_opr_ptr());
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
            _resultNodeWrapper->addForceExitNode(_subBlockNodeWrap->getForceExitNode());
        }

        (*_loopId) = (*_cntNode->getCounter());

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

    void FlowBlockLoop::add_md_log(MdLogVal* mdLogVal){

        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        mdLogVal->addVal("entNode " + _entNode->get_md_ident_val() + " " + _entNode->get_md_describe());
        mdLogVal->addVal("loopNode " + _entNode->get_md_ident_val() + " " + _loopNode->get_md_describe());
        mdLogVal->addVal("cntNode " + _cntNode->get_md_ident_val() + " " + _cntNode->get_md_describe());
        mdLogVal->addVal("exitNode " + _exitNode->get_md_ident_val() + " " + _exitNode->get_md_describe());
        mdLogVal->addVal("resultNodeWrap is" +
            _resultNodeWrapper->get_md_ident_val() + " " + _resultNodeWrapper->get_md_describe());

        auto subLog = mdLogVal->makeNewSubVal();
        _implicitFlowBlock->add_md_log(subLog);
    }

}
