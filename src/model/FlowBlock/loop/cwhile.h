//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_CWHILE_H
#define KATHRYN_CWHILE_H

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"
#include "model/FlowBlock/abstract/nodes/node.h"
#include "stateNode.h"

#define cwhile(expr) for(auto kathrynBlock = new FlowBlockCwhile(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockCwhile : public FlowBlockBase, public LoopStMacro{
    private:
        Operable* _condExpr = nullptr;
        FlowBlockBase* implicitFlowBlock = nullptr;
        bool isGetFlowBlockYet = false;


        NodeWrap*    resultNodeWrapper = nullptr;
        NodeWrap*    loopNodeWrap      = nullptr;
        PseudoNode*  exitNode          = nullptr;
        PseudoNode*  byPassExitNode    = nullptr;
        PseudoNode*  subBlockExitNode  = nullptr;
        //// it is wrap is as same as result but it is used for loop assignment

        NodeWrap* subBlockNodeWrap = nullptr;

    public:

        explicit FlowBlockCwhile(Operable& condExpr);
        virtual ~FlowBlockCwhile();

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrap* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };

}

#endif //KATHRYN_CWHILE_H
