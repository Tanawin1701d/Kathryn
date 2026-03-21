//
// Created by tanawin on 7/8/2024.
//

#ifndef PICK_H
#define PICK_H

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"
#include "pick_if.h"


#define pick for(auto kathryn_block = new FlowBlockPick(); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define pick_def kathryn_block->set_req_auto_exit();

namespace kathryn{

    class FlowBlockPick: public FlowBlockBase, public LoopStMacro{

    protected:
        bool req_auto_exit = false;

        std::vector<NodeWrap*> node_wrap_of_pick_cond_blocks;
        std::vector<FlowBlockPickCond*> pick_cond_blocks;

        NodeWrap* result_node_wrap   = nullptr;
        PseudoNode* joint_node      = nullptr;
        PseudoNode* auto_exit_node   = nullptr;
        PseudoNode* exit_node       = nullptr;

    public:

        explicit FlowBlockPick();
        ~FlowBlockPick() override;

        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        void add_con_flow_block(FlowBlockBase* con_block) override;

        /** override flow block base*/
        NodeWrap* sumarize_block() override; /// to interact from parrent block call

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;

        /** for module to build hardware component*/
        void build_hw_component() override;

        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        void set_req_auto_exit(){req_auto_exit = true;}

    };

}

#endif //PICK_H
