//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_WHILEBASE_H
#define KATHRYN_WHILEBASE_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define cwhile(expr) for(auto kathrynBlock_cwhile = new FlowBlockWhile(expr, CWHILE); kathrynBlock_cwhile->doPrePostFunction(); kathrynBlock_cwhile->step())
#define swhile(expr) for(auto kathrynBlock_swhile = new FlowBlockWhile(expr, SWHILE); kathrynBlock_swhile->doPrePostFunction(); kathrynBlock_swhile->step())

namespace kathryn{

    class FlowBlockWhile : public FlowBlockBase, public LoopStMacro{
    protected:
        ///////// condition
        bool           _fallTrue         = false;
        Operable*      _condExpr         = nullptr;
        Operable*      _purifiedCondExpr = nullptr;
        //////// block
        FlowBlockBase* implicitFlowBlock = nullptr;
        bool           isGetFlowBlockYet = false;
        //////// nodes
        NodeWrap*      resultNodeWrapper = nullptr;
        NodeWrap*      subBlockNodeWrap  = nullptr;
        Node*          conditionNode     = nullptr;
        DummyNode*     exitDummy         = nullptr;
        PseudoNode*    exitNode          = nullptr;


        //// it is wrap is as same as result but it is used for loop assignment



    public:

        explicit FlowBlockWhile(Operable& condExpr, FLOW_BLOCK_TYPE fbt);
        explicit FlowBlockWhile(bool fallTrue     , FLOW_BLOCK_TYPE fbt);
        ~FlowBlockWhile() override;

        void buildHwComponent() override;

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

        void addMdLog(MdLogVal* mdLogVal) override;
    };

}

#endif //KATHRYN_WHILEBASE_H
