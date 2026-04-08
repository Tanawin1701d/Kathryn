//
// Created by tanawin on 5/1/2567.
//

#ifndef KATHRYN_CBREAK_H
#define KATHRYN_CBREAK_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


#define sbreak          for(auto kathrynBlock = new FlowBlockSCBreak();     kathrynBlock->doPrePostFunction(); kathrynBlock->step()){}
////#define sbreakCon(expr) for(auto kathrynBlock = new FlowBlockSCBreak(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step()){}

namespace kathryn{

    class FlowBlockSCBreak : public FlowBlockBase, public LoopStMacro{
    private:
        Operable* forceExitOpr   = nullptr;
        NodeWrap* resultNodeWrap = nullptr;

        StateNode* breakNode      = nullptr;
        ////////// mark as legacy we don't support scbreak con anymore
        PseudoNode* breakCondNode = nullptr; /// incase there is condition to break node
        DummyNode* normExitNode   = nullptr;

    public:
        explicit FlowBlockSCBreak();
        explicit FlowBlockSCBreak(Operable& opr1);
        ~FlowBlockSCBreak();
        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
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


#endif //KATHRYN_CBREAK_H
