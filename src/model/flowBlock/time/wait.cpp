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
    FlowBlockCondWait::FlowBlockCondWait(Operable& exitCond)
    : FlowBlockBase(CONDWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
    _resultNodeWrap(nullptr),
    _exitCond(&exitCond),
    _purifiedExitCond(purify_condition(&exitCond)),
    _waitNode(nullptr)
    {
        //assert(exitCond != nullptr);
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
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockCondWait::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }
    /** buildHwComponent*/
    void FlowBlockCondWait::build_hw_component() {
        assert(_con_blocks.empty());
        assert(!is_there_intr_start());

        /** build node*/

        _waitNode = new WaitCondNode(_exitCond, get_clock_mode());
        add_sys_node(_waitNode);
        fill_intr_reset_to_node_if_there(_waitNode);
        fill_hold_to_node_if_there(_waitNode);
        _waitNode->set_internal_ident("waitCond"+std::to_string(get_global_id()));

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
        ret += "exitCond is " + _exitCond->castToIdent()->getIdentDebugValue() + "\n";
        ret += "_waitNode is " +_waitNode->get_md_ident_val()+ " " +_waitNode->get_md_describe() + "\n";
        return ret;
    }

    void FlowBlockCondWait::add_md_log(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + "]");
        mdLogVal->addVal("exitVal cond is " + _exitCond->castToIdent()->getIdentDebugValue());
        mdLogVal->addVal("waitNode is " + _waitNode->get_md_ident_val()+ " " +_waitNode->get_md_describe());
    }

    /***
     *
     * cycle wait
     *
     * */



    FlowBlockCycleWait::FlowBlockCycleWait(int cycleUsed)
    : FlowBlockBase(CLKWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
      _resultNodeWrap(nullptr),
      _waitNode(nullptr),
      cycle(cycleUsed),
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
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockCycleWait::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockCycleWait::build_hw_component() {
        assert(_con_blocks.empty());
        assert(!is_there_intr_start());

        /** build node*/
        if (cnt != nullptr) {
            _waitNode = new WaitCycleNode(cnt, get_clock_mode());
        }else {
            assert(cycle > 0);
            _waitNode = new WaitCycleNode(cycle, get_clock_mode());
        }
        add_sys_node(_waitNode);
        fill_intr_reset_to_node_if_there(_waitNode);
        fill_hold_to_node_if_there(_waitNode);
        _waitNode->set_internal_ident("waitCycle"+std::to_string(get_global_id()));

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
        ret += "[cnt] is " + cnt->castToIdent()->getIdentDebugValue()+ "\n";
        return ret;
    }

    void FlowBlockCycleWait::add_md_log(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + "]");
        mdLogVal->addVal( "wait Node is " + _waitNode->get_md_ident_val() + "  " + _waitNode->get_md_describe());
        mdLogVal->addVal("counter" + cnt->castToIdent()->getIdentDebugValue());
    }

}