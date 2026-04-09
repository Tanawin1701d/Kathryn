//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_IF_H
#define KATHRYN_IF_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define cif(expr) for(auto kathryn_block = new FlowBlockIf(expr, CIF); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define sif(expr) for(auto kathryn_block = new FlowBlockIf(expr, SIF); kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockElif;

    class FlowBlockIf: public FlowBlockBase, public LoopStMacro{
    private:

        FlowBlockBase*          implicitFlowBlock = nullptr;
        std::vector<NodeWrap*>  allStatement; /// include current block and else block
        std::vector<Operable*>  allCondes; /// include condition of if block and elif block except else block
        std::vector<Operable*>  allPurifiedCondes;

        Node*       condNode       = nullptr;
        PseudoNode* exitNode       = nullptr;
        NodeWrap*   resultNodeWrap = nullptr;

    public:
        explicit FlowBlockIf(Operable& cond, FLOW_BLOCK_TYPE flowBlockType);
        ~FlowBlockIf() override;

        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
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
        void add_md_log(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;
    };



}

#endif //KATHRYN_IF_H
