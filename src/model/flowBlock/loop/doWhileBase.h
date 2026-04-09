//
// Created by tanawin on 22/4/2567.
//

#ifndef KATHRYN_DOWHILEBASE_H
#define KATHRYN_DOWHILEBASE_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define cdowhile(expr) for(auto kathryn_block = new FlowBlockDowhile(expr, DOWHILE); kathryn_block->do_pre_post_function(); kathryn_block->step())


namespace kathryn{

    class FlowBlockDowhile : public FlowBlockBase, public LoopStMacro{
    protected:
        ///////// condition
        Operable*      _condExpr         = nullptr;
        Operable*      _purifiedCondExpr = nullptr;
        //////// block
        FlowBlockBase* implicitFlowBlock = nullptr;
        bool           isGetFlowBlockYet = false;
        //////// nodes
        NodeWrap*      resultNodeWrapper = nullptr;
        NodeWrap*      subBlockNodeWrap  = nullptr;
        PseudoNode*    exitNode          = nullptr;


        //// it is wrap is as same as result but it is used for loop assignment



    public:

        explicit FlowBlockDowhile(Operable& condExpr, FLOW_BLOCK_TYPE fbt);
        ~FlowBlockDowhile() override;

        void build_hw_component() override;

        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        ///////////void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        void add_md_log(MdLogVal* mdLogVal) override;
    };

}

#endif //KATHRYN_DOWHILEBASE_H
