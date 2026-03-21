//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_IF_H
#define KATHRYN_IF_H

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

#define cif(expr) for(auto kathryn_block = new FlowBlockIf(expr, CIF); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define sif(expr) for(auto kathryn_block = new FlowBlockIf(expr, SIF); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockElif;

    class FlowBlockIf: public FlowBlockBase, public LoopStMacro{
    private:

        FlowBlockBase*          implicit_flow_block = nullptr;
        std::vector<NodeWrap*>  all_statement; /// include current block and else block
        std::vector<Operable*>  all_condes; /// include condition of if block and elif block except else block
        std::vector<Operable*>  all_purified_condes;

        Node*       cond_node       = nullptr;
        PseudoNode* exit_node       = nullptr;
        NodeWrap*   result_node_wrap = nullptr;

    public:
        explicit FlowBlockIf(Operable& cond, FLOW_BLOCK_TYPE flow_block_type);
        ~FlowBlockIf() override;

        /** for controller add the local element to this sub block*/
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
        std::string get_md_describe() override;
        void add_md_log(MdLogVal *md_log_val) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;
    };



}

#endif //KATHRYN_IF_H
