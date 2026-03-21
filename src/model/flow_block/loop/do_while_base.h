//
// Created by tanawin on 22/4/2567.
//

#ifndef KATHRYN_DOWHILEBASE_H
#define KATHRYN_DOWHILEBASE_H


#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

#define cdowhile(expr) for(auto kathryn_block = new FlowBlockDowhile(expr, DOWHILE); kathryn_block->do_pre_post_function(); kathryn_block->step())


namespace kathryn{

    class FlowBlockDowhile : public FlowBlockBase, public LoopStMacro{
    protected:
        ///////// condition
        Operable*      _condExpr         = nullptr;
        Operable*      _purifiedCondExpr = nullptr;
        //////// block
        FlowBlockBase* implicit_flow_block = nullptr;
        bool           is_get_flow_block_yet = false;
        //////// nodes
        NodeWrap*      result_node_wrapper = nullptr;
        NodeWrap*      sub_block_node_wrap  = nullptr;
        PseudoNode*    exit_node          = nullptr;


        //// it is wrap is as same as result but it is used for loop assignment



    public:

        explicit FlowBlockDowhile(Operable& cond_expr, FLOW_BLOCK_TYPE fbt);
        ~FlowBlockDowhile() override;

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

#endif //KATHRYN_DOWHILEBASE_H
