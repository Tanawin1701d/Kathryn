//
// Created by tanawin on 7/8/2024.
//

#include "pick.h"

#include "model/controller/controller.h"


namespace kathryn{

    FlowBlockPick::FlowBlockPick():
       FlowBlockBase(PICK,
         {
                 {FLOW_ST_BASE_STACK},
                 FLOW_JO_SUB_FLOW,
                 true
         })
    {}

    FlowBlockPick::~FlowBlockPick(){
        delete result_node_wrap;
        delete joint_node;
        delete auto_exit_node;
        delete exit_node;
    }

    void FlowBlockPick::add_element_in_flow_block(Node* node){
        assert(false);
    }

    void FlowBlockPick::add_sub_flow_block(FlowBlockBase* sub_block){
        assert(sub_block->get_flow_type() == PICK_WHEN);
        FlowBlockBase::add_sub_flow_block(sub_block);
        pick_cond_blocks.push_back(dynamic_cast<FlowBlockPickCond*>(sub_block));
    }

    void FlowBlockPick::add_con_flow_block(FlowBlockBase* con_block){
        assert(false);
    }



    NodeWrap* FlowBlockPick::sumarize_block(){
        assert(result_node_wrap != nullptr);
        return result_node_wrap;
    }

    void FlowBlockPick::on_attach_block(){
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPick::on_detach_block(){
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPick::build_hw_component(){
        ////// summarize all block
        for (auto pick_cond_block: pick_cond_blocks){
            node_wrap_of_pick_cond_blocks.push_back(pick_cond_block->sumarize_block());
        }
        assert(!node_wrap_of_pick_cond_blocks.empty());
        assert(!pick_cond_blocks.empty());
        assert(node_wrap_of_pick_cond_blocks.size() == pick_cond_blocks.size());
        assert(_conBlocks.empty());

        /////// build start node
        joint_node = new PseudoNode(1, BITWISE_OR);
        joint_node->set_internal_ident("joint_of_pick_node" + std::to_string(get_global_id()));
        if (is_there_int_start()){
            joint_node->add_depend_node(int_nodes[INT_START], nullptr);
        }
        add_sys_node(joint_node);
        for (int sid = 0; sid < node_wrap_of_pick_cond_blocks.size(); sid++){
            node_wrap_of_pick_cond_blocks[sid]
            ->add_depend_node_to_all_node(joint_node,pick_cond_blocks[sid]->get_condition_ptr());
            node_wrap_of_pick_cond_blocks[sid]->assign_all_node();
        }

        ////// build auto exit_node if needed
        if (req_auto_exit){
            auto_exit_node = new PseudoNode(1, BITWISE_AND);
            add_sys_node(auto_exit_node);
            auto_exit_node->set_internal_ident("pick_auto_exit" + std::to_string(get_global_id()));
            Operable* all_false = nullptr;
            for (FlowBlockPickCond* fpc: pick_cond_blocks){
                assert(fpc != nullptr);
                if (all_false == nullptr){
                    all_false = &(!*fpc->get_condition_ptr());
                }else{
                    all_false = &((*all_false) & (!*fpc->get_condition_ptr()));
                }
            }
            assert(all_false != nullptr);
            auto_exit_node->add_depend_node(joint_node, all_false);
            auto_exit_node->assign();
        }

        /////// build exit node
        exit_node = new PseudoNode(1, BITWISE_OR);
        add_sys_node(exit_node);
        exit_node->set_internal_ident("pick_exit" + std::to_string(get_global_id()));
        ////// join all exit node
        for (auto & node_wrap_of_pick_cond_block : node_wrap_of_pick_cond_blocks){
            exit_node->add_depend_node(
                node_wrap_of_pick_cond_block->get_exit_node(),
                nullptr);
        }
        if (auto_exit_node != nullptr){
            exit_node->add_depend_node(auto_exit_node, nullptr);
        }

        exit_node->assign();

        ///////
        ///
        ///result node warp
        ///
        ///////

        result_node_wrap = new NodeWrap();
        result_node_wrap->add_entrace_node(joint_node);
        result_node_wrap->add_exit_node(exit_node);

        //// do force exit node
        gen_sum_force_exit_node(node_wrap_of_pick_cond_blocks);
        if (_areThereForceExit){
            result_node_wrap->add_force_exit_node(_forceExitNode);
        }

        //// cycle determiner
        NodeWrapCycleDet deter;
        deter.add_to_det(node_wrap_of_pick_cond_blocks);
        if (req_auto_exit){
            deter.add_to_det(0);
        }
        int cycle_used = deter.get_same_cycle_horizon();
        result_node_wrap->set_cycle_used(cycle_used);
    }

    void FlowBlockPick::do_pre_function() {
        on_attach_block();
    }

    void
    FlowBlockPick::do_post_function(){
        on_detach_block();
    }











}
