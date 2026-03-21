//
// Created by tanawin on 2/1/2567.
//

#include "wait.h"
#include "model/controller/controller.h"



namespace kathryn{


    /***
     *
     * condition wait
     *
     * */

    /**constructor*/
    FlowBlockCondWait::FlowBlockCondWait(Operable& exit_cond)
    : FlowBlockBase(CONDWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
    _resultNodeWrap(nullptr),
    _exitCond(&exit_cond),
    _purifiedExitCond(purify_condition(&exit_cond)),
    _waitNode(nullptr)
    {
        //assert(exit_cond != nullptr);
    }

    FlowBlockCondWait::~FlowBlockCondWait(){
        delete _waitNode;
        delete _resultNodeWrap;
    }

    NodeWrap* FlowBlockCondWait::sumarize_block() {
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockCondWait::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockCondWait::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }
    /** build_hw_component*/
    void FlowBlockCondWait::build_hw_component() {
        assert(_conBlocks.empty());
        assert(!is_there_int_start());

        /** build node*/

        _waitNode = new WaitCondNode(_exitCond, get_clock_mode());
        add_sys_node(_waitNode);
        fill_int_reset_to_node_if_there(_waitNode);
        fill_hold_to_node_if_there(_waitNode);
        _waitNode->set_internal_ident("wait_cond"+std::to_string(get_global_id()));

        /** result node wrap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->add_entrace_node(_waitNode);
        _resultNodeWrap->add_exit_node(_waitNode);

    }

    void FlowBlockCondWait::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockCondWait::do_post_function() {
        on_detach_block();
    }

    std::string FlowBlockCondWait::get_md_describe(){
        std::string ret;
        ret += "[ " + get_md_ident_val() + " ]\n";
        ret += "exit_cond is " + _exitCond->cast_to_ident()->get_ident_debug_value() + "\n";
        ret += "_waitNode is " +_waitNode->get_md_ident_val()+ " " +_waitNode->get_md_describe() + "\n";
        return ret;
    }

    void FlowBlockCondWait::add_md_log(MdLogVal *md_log_val) {
        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + "]");
        md_log_val->add_val("exit_val cond is " + _exitCond->cast_to_ident()->get_ident_debug_value());
        md_log_val->add_val("wait_node is " + _waitNode->get_md_ident_val()+ " " +_waitNode->get_md_describe());
    }

    /***
     *
     * cycle wait
     *
     * */



    FlowBlockCycleWait::FlowBlockCycleWait(int cycle_used)
    : FlowBlockBase(CLKWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
      _resultNodeWrap(nullptr),
      _waitNode(nullptr),
      cycle(cycle_used),
      cnt(nullptr)
    {
        assert(cycle > 0);
    }

    FlowBlockCycleWait::FlowBlockCycleWait(Operable& opr1)
    : FlowBlockBase(CLKWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
    _resultNodeWrap(nullptr),
    _waitNode(nullptr),
    cycle(-1),
    cnt(&opr1)
    {
        ///////assert(opr != nullptr);
    }

    FlowBlockCycleWait::~FlowBlockCycleWait(){
        delete _waitNode;
        delete _resultNodeWrap;
    }

    NodeWrap* FlowBlockCycleWait::sumarize_block() {
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockCycleWait::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockCycleWait::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockCycleWait::build_hw_component() {
        assert(_conBlocks.empty());
        assert(!is_there_int_start());

        /** build node*/
        if (cnt != nullptr) {
            _waitNode = new WaitCycleNode(cnt, get_clock_mode());
        }else {
            assert(cycle > 0);
            _waitNode = new WaitCycleNode(cycle, get_clock_mode());
        }
        add_sys_node(_waitNode);
        fill_int_reset_to_node_if_there(_waitNode);
        fill_hold_to_node_if_there(_waitNode);
        _waitNode->set_internal_ident("wait_cycle"+std::to_string(get_global_id()));

        /** result node wrap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->add_entrace_node(_waitNode);
        _resultNodeWrap->add_exit_node(_waitNode);

    }

    void FlowBlockCycleWait::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockCycleWait::do_post_function() {
        on_detach_block();
    }

    std::string FlowBlockCycleWait::get_md_describe(){
        std::string ret;
        ret += "\n";
        ret += "[_waitNode] is " + _waitNode->get_md_ident_val() + "  " + _waitNode->get_md_describe() + "\n";
        ret += "[cnt] is " + cnt->cast_to_ident()->get_ident_debug_value()+ "\n";
        return ret;
    }

    void FlowBlockCycleWait::add_md_log(MdLogVal *md_log_val) {
        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + "]");
        md_log_val->add_val( "wait Node is " + _waitNode->get_md_ident_val() + "  " + _waitNode->get_md_describe());
        md_log_val->add_val("counter" + cnt->cast_to_ident()->get_ident_debug_value());
    }

}