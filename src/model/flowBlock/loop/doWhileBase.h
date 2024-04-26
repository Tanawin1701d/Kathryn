//
// Created by tanawin on 22/4/2567.
//

#ifndef KATHRYN_DOWHILEBASE_H
#define KATHRYN_DOWHILEBASE_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define cdowhile(expr) for(auto kathrynBlock_dowhile = new FlowBlockDowhile(expr, DOWHILE); kathrynBlock_dowhile->doPrePostFunction(); kathrynBlock_dowhile->step())


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

#endif //KATHRYN_DOWHILEBASE_H
