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


#define pick for(auto kathrynBlock = new FlowBlockPick(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define pickDef kathrynBlock->setReqAutoExit();

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

        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;

        /** override flow block base*/
        NodeWrap* sumarizeBlock() override; /// to interact from parrent block call

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;

        /** for module to build hardware component*/
        void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

        void setReqAutoExit(){reqAutoExit = true;}

        /** debug*/
        virtual void addMdLog(MdLogVal* mdLogVal) override;

    };

}

#endif //PICK_H
