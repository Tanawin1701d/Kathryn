//
// Created by tanawin on 4/12/2566.
//

#include "par.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockPar::FlowBlockPar(FLOW_BLOCK_TYPE fb_type):
    FlowBlockBase(fb_type,
      {
              {FLOW_ST_BASE_STACK,
               FLOW_ST_PATTERN_STACK
              },
              FLOW_JO_SUB_FLOW,
              true
      })
      {
        assert((fb_type == PARALLEL_AUTO_SYNC) ||
               (fb_type == PARALLEL_NO_SYN));
    }

    FlowBlockPar::~FlowBlockPar(){
        delete result_node_wrap;
        delete basic_st_node;
        delete syn_node;
        delete pseudo_exit_node;
    }

    NodeWrap*
    FlowBlockPar::sumarize_block() {
        assert(result_node_wrap != nullptr);
        return result_node_wrap;
    }


    void
    FlowBlockPar::on_attach_block(){
        ctrl->on_attach_flowBlock(this);
    }

    void
    FlowBlockPar::on_detach_block() {
        ctrl->on_detach_flowBlock(this);
    }

    void
    FlowBlockPar::build_hw_component() {
        mf_assert((!_basicNodes.empty()) || (!_subBlocks.empty()),
                 "par_block has no assignment"
                 );
        assert(_conBlocks.empty());

        /**
         *
         * build node for basic assignment
         *
         * */
        if (!_basicNodes.empty()){
            basic_st_node = new StateNode(get_clock_mode());
            basic_st_node->set_internal_ident("parStateReg_" + std::to_string(get_global_id()));
            add_sys_node(basic_st_node);
            fill_int_reset_to_node_if_there(basic_st_node);
            fill_hold_to_node_if_there(basic_st_node);
            /** add basic assignment to depend on state_node*/
            for (auto nd : _basicNodes){
                assert(nd->get_node_type() == ASM_NODE);
                basic_st_node->add_slave_asm_node((AsmNode*)nd);
            }
        }


        /**
         *
         * gen node wrap
         *
         * */
        for (auto fb : _subBlocks){
            NodeWrap* nw = fb->sumarize_block();
            assert(nw != nullptr);
            node_wrap_of_sub_block.push_back(nw);
        }

        /**
         * gen force exit node
         * */
        gen_sum_force_exit_node(node_wrap_of_sub_block);

        /**
         *
         * determine basic cycle used
         *
         * */
        NodeWrapCycleDet cycle_det;
        if (basic_st_node != nullptr)
            cycle_det.add_to_det(basic_st_node);
        cycle_det.add_to_det(node_wrap_of_sub_block);
        cycle_used = cycle_det.get_max_cycle_horizon();
        /**
         * build result node wrap entrance
         * */
        /*** entrance node management*/
        result_node_wrap = new NodeWrap();
        if (basic_st_node != nullptr) {
            result_node_wrap->add_entrace_node(basic_st_node);
            if (is_there_int_start()){
                basic_st_node->add_depend_node(int_nodes[INT_START], nullptr);
            }
        }
        for (auto nw : node_wrap_of_sub_block){
            result_node_wrap->transfer_ent_node_from(nw);
            if (is_there_int_start()){
                nw->add_depend_node_to_all_node(int_nodes[INT_START]);
            }
        }
        /** scan for master join block**/
        master_join_flow_block = scan_master_join_sub_block();
        /**
         * assemble the result node wrap focused on synchronization and exit parameter
         * **/
        build_sync_node();
        assign_exit_to_rnw();
        assign_cycle_used_to_rnw();
        assign_force_exit_to_rnw();
    }


    void FlowBlockPar::assign_cycle_used_to_rnw(){
        result_node_wrap->set_cycle_used(cycle_used);
    }

    void FlowBlockPar::assign_force_exit_to_rnw() {
        if (_areThereForceExit){
            result_node_wrap->add_force_exit_node(_forceExitNode);
        }
    }

    void
    FlowBlockPar::do_pre_function() {
        on_attach_block();
    }

    void
    FlowBlockPar::do_post_function(){
        on_detach_block();
    }

    std::string FlowBlockPar::get_md_describe() {
        std::string ret;

        ret += "basic_state_node is " +
                ((basic_st_node != nullptr) ?
                basic_st_node->get_md_ident_val() + "  " + basic_st_node->get_md_describe():
                ""
                ) +"\n";

        ret += "syn_node is " +
                ((syn_node != nullptr) ?
                syn_node->get_md_ident_val() + "  " + syn_node->get_md_describe():
                "") + "\n";

        ret += "pseudo_exit_node is " +
                ((pseudo_exit_node != nullptr) ?
                pseudo_exit_node->get_md_ident_val() + "  " + pseudo_exit_node->get_md_describe():
                "") + "\n";

        ret += get_md_describe_recur();
        ret += "\n";

        return ret;
    }

    void FlowBlockPar::add_md_log(MdLogVal *md_log_val) {


        md_log_val->add_val("[ " + FlowBlockBase::get_md_ident_val() + " ]");

        md_log_val->add_val("basic_state_node is " +
                         ((basic_st_node != nullptr) ?
                          basic_st_node->get_md_ident_val() + "  " + basic_st_node->get_md_describe():
                          ""
                         ));

        md_log_val->add_val("syn_node is " +
                         ((syn_node != nullptr) ?
                          syn_node->get_md_ident_val() + "  " + syn_node->get_md_describe():
                          ""));

        md_log_val->add_val("pseudo_exit_node is " +
                         ((pseudo_exit_node != nullptr) ?
                          pseudo_exit_node->get_md_ident_val() + "  " + pseudo_exit_node->get_md_describe():
                          ""));

        Node* exit_node = result_node_wrap->get_exit_node();
        md_log_val->add_val("exit node is " +
                        ( (exit_node != nullptr) ?
                            exit_node->get_md_ident_val() + "  " + exit_node->get_md_describe():
                            ""));

        if (result_node_wrap->is_there_force_exit_node()){
            md_log_val->add_val("force_exit is " + result_node_wrap->get_force_exit_node_ptr()->get_md_ident_val() +
                             "  " +
                             result_node_wrap->get_force_exit_node_ptr()->get_md_describe());
        }

        add_md_log_recur(md_log_val);
    }

    /**
     *
     *
     * parallel block auto
     *
     *
     * */

    void FlowBlockParAuto::build_sync_node() {
        int amt_block = ((basic_st_node != nullptr) ? 1 : 0) +
                        (int)(node_wrap_of_sub_block.size());
        /** build syn node if need*/
        if (
            (master_join_flow_block == nullptr) && //// no user defined exit
            (cycle_used == IN_CONSIST_CYCLE_USED) && ///// can't know the longest subblock
             (amt_block > 1) /** incase amt_block == 1 we don't have to sync*/
                ){
            /////// syn reg needed
            int syn_size = amt_block;
            syn_node = new SynNode(syn_size, get_clock_mode());
            add_sys_node(syn_node);
            fill_int_reset_to_node_if_there(syn_node);
            ///////[warning] this time we ensure that gensumforce_exit is declared
            if(_forceExitNode){
                syn_node->set_force_exit_event(_forceExitNode);
            }
            syn_node->set_internal_ident(
                    "parSynNode_" +
                    std::to_string(get_global_id())
                    );
            /**syn node don't need to specify join operation due to it used own logic or*/
            if (basic_st_node != nullptr){
                syn_node->add_depend_node(basic_st_node, nullptr);
            }
            for (auto nw : node_wrap_of_sub_block){
                syn_node->add_depend_node(nw->get_exit_node(), nullptr);
            }
            ////// assign sync reg and sync node don't have to set join op because
            /////////// sync register will handle it
            syn_node->assign();
        }

    }

    void FlowBlockParAuto::assign_exit_to_rnw() {

        int amt_block = ((basic_st_node != nullptr) ? 1 : 0) +
                        (int)(node_wrap_of_sub_block.size());

        if (syn_node != nullptr){
            result_node_wrap->add_exit_node(syn_node);
        }else if (master_join_flow_block != nullptr){ //// master_join is come from user declaration
            NodeWrap* joinner_node_wrap = master_join_flow_block->sumarize_block();
            Node* exit_node = joinner_node_wrap->get_exit_node();
            assert(exit_node != nullptr);
            result_node_wrap->add_exit_node(exit_node);
        }else{
            /** get Match allow nullptr*/
            Node* exit_node = nullptr;
            if (cycle_used >= 0){    /////// can determine cycle
                exit_node = get_match_node_from_nds_or_nws_ptr({basic_st_node},
                                                    node_wrap_of_sub_block,
                                                    cycle_used);
            }else{ /////// cannot determine but have only one
                assert(amt_block == 1); //// in > 1
                exit_node = get_any_node_from_nds_or_nws_ptr({basic_st_node},
                                                  node_wrap_of_sub_block);
            }
            assert(exit_node != nullptr);
            result_node_wrap->add_exit_node(exit_node);
        }

    }


    /**
     *
     *
     * parallel block no sync
     *
     *
     * */
    void FlowBlockParNoSync::assign_exit_to_rnw() {

        int amt_block = ((basic_st_node != nullptr) ? 1 : 0) +
                        (int)(node_wrap_of_sub_block.size());

        /** get Match allow nullptr*/
        Node* exit_node = nullptr;
        if (cycle_used >= 0){
            exit_node = get_match_node_from_nds_or_nws_ptr({basic_st_node},
                                                node_wrap_of_sub_block,
                                                cycle_used);
        }else if (amt_block == 1){
            assert(amt_block == 1);
            exit_node = get_any_node_from_nds_or_nws_ptr({basic_st_node},
                                              node_wrap_of_sub_block);
        }else{
            assert(amt_block > 1);
            pseudo_exit_node = new PseudoNode(1, BITWISE_OR);
            add_sys_node(pseudo_exit_node);
            if (basic_st_node != nullptr)
                pseudo_exit_node->add_depend_node(basic_st_node, nullptr);
            for (auto nw : node_wrap_of_sub_block){
                pseudo_exit_node->add_depend_node(nw->get_exit_node(), nullptr);
            }
            pseudo_exit_node->assign();
            exit_node  = pseudo_exit_node;
        }
        assert(exit_node != nullptr);
        result_node_wrap->add_exit_node(exit_node);
    }
}
