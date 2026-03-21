//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_WHILEBASE_H
#define KATHRYN_WHILEBASE_H

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

#define cwhile(expr) for(auto kathryn_block = new FlowBlockWhile(expr, CWHILE); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define swhile(expr) for(auto kathryn_block = new FlowBlockWhile(expr, SWHILE); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockWhile : public FlowBlockBase, public LoopStMacro{
    protected:
        ///////// condition
        bool           _fallTrue         = false;
        Operable*      _condExpr         = nullptr;
        Operable*      _purifiedCondExpr = nullptr;
        //////// block
        FlowBlockBase* implicit_flow_block = nullptr;
        bool           is_get_flow_block_yet = false;
        //////// nodes
        NodeWrap*      result_node_wrapper = nullptr;
        NodeWrap*      sub_block_node_wrap  = nullptr;
        Node*          condition_node     = nullptr;
        DummyNode*     exit_dummy         = nullptr;
        PseudoNode*    exit_node          = nullptr;


        //// it is wrap is as same as result but it is used for loop assignment



    public:

        explicit FlowBlockWhile(Operable& cond_expr, FLOW_BLOCK_TYPE fbt);
        explicit FlowBlockWhile(bool fall_true     , FLOW_BLOCK_TYPE fbt);
        ~FlowBlockWhile() override;

        void build_hw_component() override;

        /** for controller add the local element to this sub block*/
        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        ///////////void build_hw_component() override;

        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        void add_md_log(MdLogVal* md_log_val) override;
    };

}

#endif //KATHRYN_WHILEBASE_H
