//
// Created by tanawin on 7/8/2024.
//

#include "pick.h"

#include <model/controller/controller.h>


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
        delete resultNodeWrap;
        delete jointNode;
        delete autoExitNode;
        delete exitNode;
    }

    void FlowBlockPick::add_basic_node(Node* node){
        assert(false);
    }

    void FlowBlockPick::add_sub_flow_block(FlowBlockBase* subBlock){
        assert(subBlock->get_flow_type() == PICK_WHEN);
        FlowBlockBase::add_sub_flow_block(subBlock);
        pickCondBlocks.push_back(dynamic_cast<FlowBlockPickCond*>(subBlock));
    }

    void FlowBlockPick::add_con_flow_block(FlowBlockBase* conBlock){
        assert(false);
    }



    NodeWrap* FlowBlockPick::sumarize_block(){
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockPick::on_attach_block(){
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockPick::on_detach_block(){
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPick::build_hw_component(){
        ////// summarize all block
        for (auto pickCondBlock: pickCondBlocks){
            nodeWrapOfPickCondBlocks.push_back(pickCondBlock->sumarize_block());
        }
        assert(!nodeWrapOfPickCondBlocks.empty());
        assert(!pickCondBlocks.empty());
        assert(nodeWrapOfPickCondBlocks.size() == pickCondBlocks.size());
        assert(_con_blocks.empty());

        /////// build start node
        jointNode = new PseudoNode(1, BITWISE_OR);
        jointNode->set_internal_ident("jointOfPickNode" + std::to_string(get_global_id()));
        if (is_there_intr_start()){
            jointNode->add_depend_node(_int_nodes[INT_START], nullptr);
        }
        add_sys_node(jointNode);
        for (int sid = 0; sid < nodeWrapOfPickCondBlocks.size(); sid++){
            nodeWrapOfPickCondBlocks[sid]
            ->add_depend_node_to_all_node(jointNode,pickCondBlocks[sid]->getCondition());
            nodeWrapOfPickCondBlocks[sid]->assign_all_node();
        }

        ////// build auto exitNode if needed
        if (reqAutoExit){
            autoExitNode = new PseudoNode(1, BITWISE_AND);
            add_sys_node(autoExitNode);
            autoExitNode->set_internal_ident("pickAutoExit" + std::to_string(get_global_id()));
            Operable* allFalse = nullptr;
            for (FlowBlockPickCond* fpc: pickCondBlocks){
                assert(fpc != nullptr);
                if (allFalse == nullptr){
                    allFalse = &(!*fpc->getCondition());
                }else{
                    allFalse = &((*allFalse) & (!*fpc->getCondition()));
                }
            }
            assert(allFalse != nullptr);
            autoExitNode->add_depend_node(jointNode, allFalse);
            autoExitNode->assign();
        }

        /////// build exit node
        exitNode = new PseudoNode(1, BITWISE_OR);
        add_sys_node(exitNode);
        exitNode->set_internal_ident("pickExit" + std::to_string(get_global_id()));
        ////// join all exit node
        for (auto & nodeWrapOfPickCondBlock : nodeWrapOfPickCondBlocks){
            exitNode->add_depend_node(
                nodeWrapOfPickCondBlock->get_exit_node(),
                nullptr);
        }
        if (autoExitNode != nullptr){
            exitNode->add_depend_node(autoExitNode, nullptr);
        }

        exitNode->assign();

        ///////
        ///
        ///result node warp
        ///
        ///////

        resultNodeWrap = new NodeWrap();
        resultNodeWrap->add_entrace_node(jointNode);
        resultNodeWrap->add_exit_node(exitNode);

        //// do force exit node
        gen_sum_force_exit_node(nodeWrapOfPickCondBlocks);
        if (_are_there_force_exit){
            resultNodeWrap->add_force_exit_node(_force_exit_node);
        }

        //// cycle determiner
        NodeWrapCycleDet deter;
        deter.add_to_det(nodeWrapOfPickCondBlocks);
        if (reqAutoExit){
            deter.add_to_det(0);
        }
        int cycleUsed = deter.get_same_cycle_horizon();
        resultNodeWrap->set_cycle_used(cycleUsed);
    }

    void FlowBlockPick::do_pre_function() {
        on_attach_block();
    }

    void
    FlowBlockPick::do_post_function(){
        on_detach_block();
    }











}
