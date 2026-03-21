//
// Created by tanawin on 5/1/2567.
//

#ifndef KATHRYN_CBREAK_H
#define KATHRYN_CBREAK_H


#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"


#define sbreak          for(auto kathryn_block = new FlowBlockSCBreak();     kathryn_block->do_pre_post_function(); kathryn_block->step()){}
////#define sbreak_con(expr) for(auto kathryn_block = new FlowBlockSCBreak(expr); kathryn_block->do_pre_post_function(); kathryn_block->step()){}

namespace kathryn{

    class FlowBlockSCBreak : public FlowBlockBase, public LoopStMacro{
    private:
        Operable* force_exit_opr   = nullptr;
        NodeWrap* result_node_wrap = nullptr;

        StateNode* break_node      = nullptr;
        ////////// mark as legacy we don't support scbreak con anymore
        PseudoNode* break_cond_node = nullptr; /// incase there is condition to break node
        DummyNode* norm_exit_node   = nullptr;

    public:
        explicit FlowBlockSCBreak();
        explicit FlowBlockSCBreak(Operable& opr1);
        ~FlowBlockSCBreak();
        /** for controller add the local element to this sub block*/
        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal *md_log_val) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;
    };

}


#endif //KATHRYN_CBREAK_H
