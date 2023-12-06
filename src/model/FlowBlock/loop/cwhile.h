//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_CWHILE_H
#define KATHRYN_CWHILE_H

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"

#define cwhile for(auto kathrynBlock = new FlowBlockCwhile(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockCwhile : public FlowBlockBase, public LoopStMacro{
    private:
        LoopStMacro* toDoPostBlock = nullptr;
        expression* _condExpr = nullptr;
        bool isGetFlowBlockYet = false;

        NodeWrapper* resultNodeWrapper = nullptr;

    public:

        explicit FlowBlockCwhile(expression& condExpr);
        virtual ~FlowBlockCwhile();

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrapper* sumarizeBlock() override;

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
