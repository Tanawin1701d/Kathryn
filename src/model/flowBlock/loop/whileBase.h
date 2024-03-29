//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_WHILEBASE_H
#define KATHRYN_WHILEBASE_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"



namespace kathryn{

    class FlowBlockWhileBase : public FlowBlockBase, public LoopStMacro{
    protected:
        Operable*      _condExpr         = nullptr;
        Operable*      _purifiedCondExpr = nullptr;
        FlowBlockBase* implicitFlowBlock = nullptr;
        bool           isGetFlowBlockYet = false;
        NodeWrap*      resultNodeWrapper = nullptr;
        NodeWrap*      subBlockNodeWrap  = nullptr;
        PseudoNode*    exitNode          = nullptr;


        //// it is wrap is as same as result but it is used for loop assignment



    public:

        explicit FlowBlockWhileBase(Operable& condExpr, FLOW_BLOCK_TYPE fbt);
        ~FlowBlockWhileBase() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrap* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        ///////////void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };

}

#endif //KATHRYN_WHILEBASE_H
