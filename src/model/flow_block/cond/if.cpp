//
// Created by tanawin on 6/12/2566.
//

#include "if.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockIf::FlowBlockIf(Operable& cond,
                             FLOW_BLOCK_TYPE flow_block_type):
    FlowBlockBase(flow_block_type,
       {
               {FLOW_ST_BASE_STACK,
                             FLOW_ST_HEAD_COND_STACK},
               FLOW_JO_SUB_FLOW,
               true
       }){
        assert(flow_block_type == CIF || flow_block_type == SIF);
        all_condes.push_back(&cond);
        all_purified_condes.push_back(purify_condition(&cond));
    }

    FlowBlockIf::~FlowBlockIf(){
        delete cond_node;
        delete exit_node;
        delete result_node_wrap;
    }

    void FlowBlockIf::add_element_in_flow_block(Node *node) {
        assert(false); /** this flow type will generate single par or seq block*/
    }

    void FlowBlockIf::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(sub_block != nullptr);
        FlowBlockBase::add_sub_flow_block(sub_block);
    }

    void FlowBlockIf::add_con_flow_block(FlowBlockBase* con_block){
        assert(con_block != nullptr);
        assert(con_block->get_flow_type() == CSELIF ||
               con_block->get_flow_type() == CSELSE);
        /*** convert to elif block*/
        FlowBlockElif* elif_block = (FlowBlockElif*)con_block;
        /*** call base function*/
        FlowBlockBase::add_con_flow_block(elif_block);
        /** push to if-else concern element*/
        /// all_statement.push_back(elif_block->sumarize_block());
        if (elif_block->get_condition_ptr() != nullptr) {
            all_condes.push_back(elif_block->get_condition_ptr());
            all_purified_condes.push_back(
                    purify_condition(elif_block->get_condition_ptr())
            );
        }

        assert(con_block->get_sub_blocks()[0] != nullptr);
    }

    NodeWrap* FlowBlockIf::sumarize_block() {
        assert(result_node_wrap != nullptr);
        return result_node_wrap;
    }

    void FlowBlockIf::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicit_flow_block = sb;
        sb->on_attach_block();
    }

    void FlowBlockIf::on_detach_block() {
        assert(implicit_flow_block != nullptr);
        implicit_flow_block->on_detach_block();
        ////// we will hold this end block will handle it
        set_lazy_delete();

        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockIf::build_hw_component() {
        /**summarize all block*/
        assert(_subBlocks.size() == 1);
        all_statement.push_back(_subBlocks[0]->sumarize_block());
        for (auto con_flow_block: _conBlocks){
            all_statement.push_back(con_flow_block->sumarize_block());
        }
        assert(!all_condes.empty());
        assert(all_purified_condes.size() == all_condes.size());
        assert(!all_statement.empty());

        /***
         *
         * initialize node
         *
         * */

        if (get_flow_type() == CIF){
            cond_node = new PseudoNode(1, BITWISE_OR);
            cond_node->set_internal_ident("cif_node" + std::to_string(get_global_id()));
        }else if(get_flow_type() == SIF){
            cond_node = new StateNode(get_clock_mode());
            cond_node->set_internal_ident("sif_node" + std::to_string(get_global_id()));
            fill_int_reset_to_node_if_there(cond_node);
            fill_hold_to_node_if_there(cond_node);
        }else{assert(false);}
        add_sys_node(cond_node);

        if (is_there_int_start()){
            cond_node->add_depend_node(int_nodes[INT_START], nullptr);
        }


        exit_node = new PseudoNode(1, BITWISE_OR);
        add_sys_node(exit_node);
        exit_node->set_internal_ident("if_exit_node" + std::to_string(get_global_id()));
        result_node_wrap = new NodeWrap();


        /***
         *
         * subblock and Exit assign
         *
         * */

        /**add condition to state*/
        Operable* prev_false = &(~(*all_purified_condes[0]));
        /** assign first first if*/
        all_statement[0]->add_depend_node_to_all_node( cond_node,all_purified_condes[0]);
        all_statement[0]->assign_all_node();
        exit_node->add_depend_node(all_statement[0]->get_exit_node(), nullptr);


        int statement_id = 1;
        for (; statement_id < all_statement.size(); statement_id++){
            if (statement_id < all_purified_condes.size()) {
                all_statement[statement_id]->add_depend_node_to_all_node(
                        cond_node,
                        &((*all_purified_condes[statement_id]) & (*prev_false)));
                all_statement[statement_id]->assign_all_node();
                exit_node->add_depend_node(all_statement[statement_id]->get_exit_node(), nullptr);
                prev_false = &((*prev_false) & ~(*all_purified_condes[statement_id]));
            }else{
                /** case else statement*/
                assert(statement_id == (all_purified_condes.size())); /// check no ambiguous statement
                all_statement[statement_id]->add_depend_node_to_all_node(
                        cond_node,
                        prev_false);
                all_statement[statement_id]->assign_all_node();
                exit_node->add_depend_node(all_statement[statement_id]->get_exit_node(), nullptr);
            }
        }



        if (all_statement.size() == all_purified_condes.size()){
            /** there is no else node*/
            /** prev false is ready*/
            exit_node->add_depend_node(cond_node, prev_false);
        }
        exit_node->assign();

        /**
         *
         * result node wrap
         *
         *
         * */


        result_node_wrap->add_entrace_node(cond_node);
        result_node_wrap->add_exit_node(exit_node);

        /**force exit condition*/
        gen_sum_force_exit_node(all_statement);
        if (_areThereForceExit)
            result_node_wrap->add_force_exit_node(_forceExitNode);

        /**
         *
         *
         * cycle determiner
         *
         *
         * */
        NodeWrapCycleDet deter;
        deter.add_to_det(all_statement);
        if(all_statement.size() == all_purified_condes.size()){
            /** there is zero state node*/
            deter.add_to_det(0);
        }

        /**cycle determiner for node wrap*/
        int cycle_used = deter.get_same_cycle_horizon();
        if (cycle_used == IN_CONSIST_CYCLE_USED){
            result_node_wrap->set_cycle_used(IN_CONSIST_CYCLE_USED);
        }else if (get_flow_type() == CIF){
            result_node_wrap->set_cycle_used(cycle_used);
        }else if (get_flow_type() == SIF){
            result_node_wrap->set_cycle_used(cycle_used+1);
        }else{
            assert(false);
        }

    }


    std::string FlowBlockIf::get_md_describe() {
        std::string ret;
        ret += "[ " + FlowBlockBase::get_md_ident_val() +" ]\n";
        ret += "exit_node is " + ((exit_node != nullptr) ? exit_node->get_md_ident_val()+ "  " + exit_node->get_md_describe(): "") + "\n";
        ret += "[implicit_flow_block]" + implicit_flow_block->get_md_describe() + "\n";
        return ret;
    }

    void FlowBlockIf::add_md_log(MdLogVal *md_log_val) {
        assert(all_condes.size() == all_purified_condes.size());
        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() +" ]");
        int cnt = 0;
        if (result_node_wrap->is_there_force_exit_node()){
            md_log_val->add_val("force_exit is " + result_node_wrap->get_force_exit_node_ptr()->get_md_ident_val() +
                             "  " +
                             result_node_wrap->get_force_exit_node_ptr()->get_md_describe());
        }
        for (auto sb : _subBlocks){
            std::string sub_block_header_debug = "----> subblock " + std::to_string(cnt) + " condition ";
            if (cnt < all_condes.size()) {
                sub_block_header_debug += all_condes[cnt]->cast_to_ident()->get_ident_debug_value();
            }
            md_log_val->add_val(sub_block_header_debug);

            sb->add_md_log(md_log_val->make_new_sub_val());
            cnt++;
        }
    }

    void FlowBlockIf::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockIf::do_post_function() {
        on_detach_block();
    }

}