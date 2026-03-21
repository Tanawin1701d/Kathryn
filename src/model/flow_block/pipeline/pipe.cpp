//
// Created by tanawin on 1/3/2025.
//

#include "pipe.h"

#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockPipeBase::FlowBlockPipeBase(SyncMeta& sync_meta):
    FlowBlockBase(PIPE_BLOCK,
        {
            {FLOW_ST_BASE_STACK, FLOW_ST_PIP_BLK},
            FLOW_JO_SUB_FLOW,
            true
        }),
    _syncMata(sync_meta),
    _pipeName("Pipe_"+ sync_meta.get_name()){
    }


    FlowBlockPipeBase::~FlowBlockPipeBase(){
        delete ent_node;
        delete wait_node;
        delete exit_dummy;
        delete result_node_wrap;
    }


    ///////////// ELEMENT ADDING
    void FlowBlockPipeBase::add_element_in_flow_block(Node* node){
        assert(false);
        // because the implicit par block must be assign to this system
    }

    void FlowBlockPipeBase::add_sub_flow_block(FlowBlockBase* sub_block){
        assert(sub_block != nullptr);
        assert(!is_get_flow_block_yet);
        is_get_flow_block_yet = true;
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    void FlowBlockPipeBase::add_con_flow_block(FlowBlockBase* con_block){
        ////// it can't have con block for pipeline block
        assert(false);
    }

    void FlowBlockPipeBase::add_abandon_flow_block(FlowBlockBase* abandon_block){
        ////// it can't have abandon block for pipeline block
        assert(false);
    }

    NodeWrap* FlowBlockPipeBase::sumarize_block(){
        assert(result_node_wrap != nullptr);
        return result_node_wrap;
    }

    void FlowBlockPipeBase::assign_ready_signal(){
        //////// wait signal and last stage means that it is ready
        ////(*_syncMata._syncSlaveReady) = (*ent_node->get_exit_opr_ptr());
        _syncMata.set_slave_ready(*ent_node->get_exit_opr_ptr());

        //////// set when pipeline is success
        Node* exit_node = sub_block_node_wrap->get_exit_node();
        _syncMata.set_slave_finish(*exit_node->get_exit_opr_ptr());
    }

    void FlowBlockPipeBase::build_hw_master(){
        ////// fill retrieve slave hold signal
        add_hold_signal(&_syncMata.hold_slave_signal);
        add_int_signal(INT_RESET, &_syncMata.kill_slave_signal);
        add_int_signal(INT_START, &_syncMata.start_slave_signal);

        ////// use the base  build function
        FlowBlockBase::build_hw_master();
    }


    void FlowBlockPipeBase::build_hw_component(){
        ////// try to find the activate signal
        Operable* activate_signal = nullptr;
        ////// if it auto, we have to  build the auto trigger for the other side signal
        if (is_auto_activate_pipe()){ /////// no zync source
            _syncMata.set_master_ready(make_opr_val("pipe_auto_act_" + _pipeName, 1, 1));
            //(*_syncMata._syncMasterReady) = make_opr_val("pipe_auto_act_" + _pipeName, 1, 1);
        }
        activate_signal = &_syncMata._syncMasterReady;

        ////////////// do integritry check
        assert(_conBlocks.empty());
        assert(_subBlocks.size() == 1);
        sub_block_node_wrap = _subBlocks[0]->sumarize_block();
        assert(sub_block_node_wrap != nullptr);

        //////////// init all nodes and condition

        ent_node       = new PseudoNode(1, BITWISE_OR);
        wait_node      = new StateNode(get_clock_mode());
        exit_dummy     = new DummyNode(&make_opr_val("exit_dummy",1, 0));

        ///////// add node dependency
        ent_node->add_depend_node(sub_block_node_wrap->get_exit_node(), nullptr);
        ent_node->add_depend_node(wait_node, nullptr);
        fill_int_reset_to_node_if_there(wait_node);
        fill_hold_to_node_if_there(wait_node);
        if(is_there_int_start()){
            ent_node->add_depend_node(int_nodes[INT_START], nullptr);
        }
        wait_node->add_depend_node(ent_node, &(~(*activate_signal)));
        sub_block_node_wrap->add_depend_node_to_all_node(ent_node, activate_signal);

        ////////// add system Node
        add_sys_node(ent_node);
        add_sys_node(wait_node);
        add_sys_node(exit_dummy);

        ////////// assign Node
                ////// ent wait for outer  block assign or master
        wait_node->assign();
        exit_dummy->assign();
        sub_block_node_wrap->assign_all_node();

        /////////// build result_node Wrap
        result_node_wrap = new NodeWrap();
        result_node_wrap->add_entrace_node(ent_node);
        result_node_wrap->add_exit_node(exit_dummy);

        //////////// build ready signal to tell that pipe line is ready
        assign_ready_signal();

    }

    void FlowBlockPipeBase::add_md_log(MdLogVal* md_log_val){
        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        md_log_val->add_val( "ent_node " +       ent_node->get_md_ident_val()      + " " + ent_node->get_md_describe());
        md_log_val->add_val( "wait_node " +      wait_node->get_md_ident_val()     + " " + wait_node->get_md_describe());
        md_log_val->add_val( "exit_dummy " +     exit_dummy->get_md_ident_val()    + " " + exit_dummy->get_md_describe());
        md_log_val->add_val("result_node_wrap is" +
                         result_node_wrap->get_md_ident_val() + " " + result_node_wrap->get_md_describe());

        auto sub_log = md_log_val->make_new_sub_val();
        implicit_flow_block->add_md_log(sub_log);

    }

    ///////////// FLOW BLOCK

    void FlowBlockPipeBase::on_attach_block(){
        ctrl->on_attach_flowBlock(this);
        /*** in pipe we implecitcally add parallel sub Block to system*/
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicit_flow_block = sb;
        sb->on_attach_block();
    }

    void FlowBlockPipeBase::on_detach_block(){
        assert(implicit_flow_block != nullptr);
        implicit_flow_block->on_detach_block();
        assert(is_get_flow_block_yet);
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPipeBase::do_pre_function(){
        on_attach_block();
    }

    void FlowBlockPipeBase::do_post_function(){
        on_detach_block();
    }

}
