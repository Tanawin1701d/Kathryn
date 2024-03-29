//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_ZELIF_H
#define KATHRYN_ZELIF_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


#define zelif(expr) for(auto kathrynBlock = new FlowBlockZELIF(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define zelse for(auto kathrynBlock = new FlowBlockZELIF(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockZELIF: public FlowBlockBase, public LoopStMacro{
    private:
        Operable* curCond = nullptr;

    public :
        Operable* getPurifiedCurCond();

        explicit FlowBlockZELIF(Operable& cond);
        explicit FlowBlockZELIF();
        ~FlowBlockZELIF() override;


        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* fb) override;
        NodeWrap* sumarizeBlock() override;
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
        /** override simulation*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;
    };

}

#endif //KATHRYN_ZELIF_H
