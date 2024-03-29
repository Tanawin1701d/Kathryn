//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_ZIF_H
#define KATHRYN_ZIF_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define zif(expr) for(auto kathrynBlock = new FlowBlockZIF(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    /***
       *
       * zelif-zelse will detach as conjucntion block and will be extract in zif
       * zif will extract at controller
       * */



    class FlowBlockZIF: public FlowBlockBase, public LoopStMacro{
    private:
        bool lastZelifDetected = false;
        Operable* purifiedCurCond  = nullptr;
        std::vector<Operable*> prevFalses;

    public:

        explicit FlowBlockZIF(Operable& cond);
        ~FlowBlockZIF() override;


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

#endif //KATHRYN_ZIF_H
