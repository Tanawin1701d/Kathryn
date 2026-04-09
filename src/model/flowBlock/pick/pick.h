//
// Created by tanawin on 7/8/2024.
//

#ifndef PICK_H
#define PICK_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "pickIf.h"


#define pick for(auto kathryn_block = new FlowBlockPick(); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define pickDef kathryn_block->setReqAutoExit();

namespace kathryn{

    class FlowBlockPick: public FlowBlockBase, public LoopStMacro{

    protected:
        bool reqAutoExit = false;

        std::vector<NodeWrap*> nodeWrapOfPickCondBlocks;
        std::vector<FlowBlockPickCond*> pickCondBlocks;

        NodeWrap* resultNodeWrap   = nullptr;
        PseudoNode* jointNode      = nullptr;
        PseudoNode* autoExitNode   = nullptr;
        PseudoNode* exitNode       = nullptr;

    public:

        explicit FlowBlockPick();
        ~FlowBlockPick() override;

        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        void add_con_flow_block(FlowBlockBase* conBlock) override;

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

        void setReqAutoExit(){reqAutoExit = true;}

    };

}

#endif //PICK_H
