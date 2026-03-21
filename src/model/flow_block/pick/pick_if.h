//
// Created by tanawin on 7/8/2024.
//

#ifndef KATHRYN_PICKIF_H
#define KATHRYN_PICKIF_H

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

#define pif(expr) for(auto kathryn_block = new FlowBlockPickCond(expr); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{


    class FlowBlockPickCond: public FlowBlockBase, public LoopStMacro{

    protected:

        FlowBlockBase* implicit_flow_block = nullptr;
        Operable*      condition = nullptr;

        NodeWrap*      result_node_wrapper = nullptr;

    public:

        explicit FlowBlockPickCond(Operable& cond);
        ~FlowBlockPickCond() override;

        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        void add_con_flow_block(FlowBlockBase* fb) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        [[nodiscard]]
        Operable*get_condition_ptr() const{
            return condition;
        }
    };


}

#endif //PICKIF_H
