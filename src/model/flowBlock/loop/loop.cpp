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
    _loopId(new expression(cal_bit_used_in_counter(_loopCount))){


    }

    FlowBlockLoop::~FlowBlockLoop() {
        delete _resultNodeWrapper;
        delete _entNode;
        delete _loopNode;
        delete _exitNode;
    }


    void FlowBlockLoop::build_hw_component(){
        assert(_con_blocks.empty());
        assert(_sub_blocks.size() == 1);
        _subBlockNodeWrap = _sub_blocks[0]->sumarize_block();
        assert(_subBlockNodeWrap != nullptr);


        _entNode = new PseudoNode(1, BITWISE_OR);
        _entNode->set_internal_ident("cEntNode" + std::to_string(get_global_id()));
        add_sys_node(_entNode);

        _loopNode = new PseudoNode(1, BITWISE_OR);
        _loopNode->set_internal_ident("cLoopNode" + std::to_string(get_global_id()));
        add_sys_node(_loopNode);

        _cntNode = new CounterNode(_loopCount, get_clock_mode());
        _cntNode->set_internal_ident("countNode" + std::to_string(get_global_id()));
        add_sys_node(_cntNode);

        _exitNode = new PseudoNode(1, BITWISE_AND);
        _exitNode->set_internal_ident("cExitNode" + std::to_string(get_global_id()));
        add_sys_node(_exitNode);


        ////// handle start signal
        if(is_there_intr_start()){
            _entNode->add_depend_node(_int_nodes[INT_START], nullptr);
        }
        ////// no need to reset or hold the system


        ////// loop node
        _loopNode->add_depend_node(_entNode, nullptr);
        _loopNode->add_depend_node(_subBlockNodeWrap->get_exit_node(),
                                 &(~(*_cntNode->get_exit_opr_ptr())));
        _loopNode->assign();

        ////// counter Node
        _cntNode->add_depend_node(_entNode, nullptr);
        _cntNode->make_inc_counter_event(_subBlockNodeWrap->get_exit_node());
        _cntNode->assign();

        ////// exit node
        _exitNode->add_depend_node(_subBlockNodeWrap->get_exit_node(),
                                 _cntNode->get_exit_opr_ptr());
        _exitNode->assign();

        /////// sub block trigger
        _subBlockNodeWrap->add_depend_node_to_all_node(_loopNode, nullptr);
        _subBlockNodeWrap->assign_all_node();


        _resultNodeWrapper = new NodeWrap();
        _resultNodeWrapper->add_entrace_node(_entNode);
        _resultNodeWrapper->add_exit_node(_exitNode);
        if (_subBlockNodeWrap->get_cycle_used() != IN_CONSIST_CYCLE_USED){
            _resultNodeWrapper->set_cycle_used(_subBlockNodeWrap->get_cycle_used() * _loopCount);
        }
        if (_subBlockNodeWrap->is_there_force_exit_node()){
            _resultNodeWrapper->add_force_exit_node(_subBlockNodeWrap->get_force_exit_node());
        }

        (*_loopId) = (*_cntNode->get_counter());

    }



    void FlowBlockLoop::add_basic_node(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockLoop::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!_isGetFlowBlockYet);
        _isGetFlowBlockYet = true;
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    NodeWrap* FlowBlockLoop::sumarize_block() {
        assert(_resultNodeWrapper != nullptr);
        return _resultNodeWrapper;
    }

    void FlowBlockLoop::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        _implicitFlowBlock = sb;
        sb->on_attach_block();
    }

    void FlowBlockLoop::on_detach_block() {
        assert(_implicitFlowBlock != nullptr);
        _implicitFlowBlock->on_detach_block();
        assert(_isGetFlowBlockYet);
        _ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockLoop::do_pre_function() {
        on_attach_block();
    }
    void FlowBlockLoop::do_post_function() {
        on_detach_block();
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
