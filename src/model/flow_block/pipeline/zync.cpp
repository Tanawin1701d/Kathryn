//
// Created by tanawin on 13/9/25.
//

#include "zync.h"

#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockZyncBase::FlowBlockZyncBase(
        SyncMeta& sync_meta, Operable* accept_cond):
    FlowBlockBase(PIPE_BLOCK,
        {      /////// if there is change, please do not forget to fix the constructor below
            {FLOW_ST_BASE_STACK, FLOW_ST_PIP_BLK},
            FLOW_JO_SUB_FLOW,
            true

        }),
    _syncMeta(sync_meta),
    _acceptCond(accept_cond){}

    FlowBlockZyncBase::~FlowBlockZyncBase(){

        delete prep_send_node;
        delete exit_node;
        delete result_node_wrap;
    }

    void FlowBlockZyncBase::assign_ready_signal(){
        assert(prep_send_node != nullptr);
        //_syncMeta->set_master_ready()
        Operable* ready2Sync = add_logic_with_output(prep_send_node->get_exit_opr_ptr(), _acceptCond, BITWISE_AND);
        _syncMeta.set_master_ready(*ready2Sync);
    }

    void FlowBlockZyncBase::add_sub_flow_block    (FlowBlockBase* sub_block){
        assert(false);
    }
    void FlowBlockZyncBase::add_con_flow_block    (FlowBlockBase* con_block){
        assert(false);
    }

    NodeWrap* FlowBlockZyncBase::sumarize_block(){
        assert(result_node_wrap != nullptr);
        return result_node_wrap;
    }

    void FlowBlockZyncBase::on_attach_block(){
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZyncBase::on_detach_block(){
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZyncBase::build_hw_master(){

        add_hold_signal(&_syncMeta.hold_master_signal);
        add_int_signal(INT_RESET, &_syncMeta.kill_master_signal);

        FlowBlockBase::build_hw_master();
    }


    void FlowBlockZyncBase::build_hw_component(){
        assert(_conBlocks.empty());
        assert(_subBlocks.empty());
        //assert(_syncMeta->_syncMatched != nullptr);

        /** init all nodes and condition*/
        prep_send_node = new StateNode(get_clock_mode());
        exit_node     = new PseudoNode(1, BITWISE_AND);

        ////// if it auto we have to build the auto trigger for
        if (is_auto_activate_pipe()){
            Val& auto_activate_signal = make_opr_val("zync_auto_act_" + _zyncName, 1, 1);
            _syncMeta.set_slave_ready(auto_activate_signal);
            _syncMeta.set_slave_finish(auto_activate_signal);
        }
        /** prep_send_node*/
        std::string debug_name = "zyncBlk_" + _syncMeta.get_name();
        prep_send_node->set_internal_ident("zyncBlk_" + debug_name);
        fill_int_reset_to_node_if_there(prep_send_node);
        fill_hold_to_node_if_there    (prep_send_node);
        /** assign assignment node*/
        Operable* ready_final = nullptr;
        ready_final = _acceptCond;
        ready_final = add_logic_with_output(&_syncMeta._syncSlaveReady, ready_final, BITWISE_AND);

        assert(ready_final != nullptr);
        Operable* not_ready_final = &(~(*ready_final));

        prep_send_node->add_depend_node(prep_send_node, not_ready_final);
            /** add slave assignment node*/
        for (auto nd : _basicNodes){
            assert(nd->get_node_type() == ASM_NODE);
            prep_send_node->add_slave_asm_node((AsmNode*)nd, ready_final);
        }

        /** exit Node*/
        exit_node->add_depend_node(prep_send_node, ready_final);
        /** assign node*/
        exit_node->assign();
        /** add system node*/
        add_sys_node(prep_send_node);
        add_sys_node(exit_node);
        /** result_node Wrap*/
        result_node_wrap = new NodeWrap();
        result_node_wrap->add_entrace_node(prep_send_node);
        result_node_wrap->add_exit_node(exit_node);

        /** assign the ready signal*/
        assign_ready_signal();

    }

    void FlowBlockZyncBase::add_md_log(MdLogVal* md_log_val){
        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        md_log_val->add_val(prep_send_node->get_md_ident_val() + " " + prep_send_node->get_md_describe());
        md_log_val->add_val(exit_node->get_md_ident_val() + " " + exit_node->get_md_describe());
        md_log_val->add_val("result_node_wrap is" +
                         result_node_wrap->get_md_ident_val() + " " + result_node_wrap->get_md_describe());
    }

    void FlowBlockZyncBase::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockZyncBase::do_post_function(){
        on_detach_block();
    }

}