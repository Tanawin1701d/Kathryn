//
// Created by tanawin on 2/1/2567.
//

#ifndef KATHRYN_WAIT_H
#define KATHRYN_WAIT_H

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/nodes/node.h"
#include "model/FlowBlock/abstract/nodes/waitNode.h"
#include "model/FlowBlock/abstract/loopStMacro.h"

#define scWait( cond) for(auto kathrynBlock = new FlowBlockCondWait(cond);   kathrynBlock->doPrePostFunction(); kathrynBlock->step()){};
#define syWait(cycle) for(auto kathrynBlock = new FlowBlockCycleWait(cycle); kathrynBlock->doPrePostFunction(); kathrynBlock->step()){};

namespace kathryn{

    /**
     *
     * conditional wait flow block
     *
     * */


    class FlowBlockCondWait : public FlowBlockBase, public LoopStMacro{
    protected:
        /**result node wrap to sumarize this block*/
        NodeWrap* _resultNodeWrap = nullptr;
        /** exit condition*/
        Operable* _exitCond = nullptr;
        Operable* _purifiedExitCond = nullptr;
        /** represent node*/
        WaitCondNode* _waitNode = nullptr;
    public:

        explicit FlowBlockCondWait(Operable& exitCond);
        ~FlowBlockCondWait() override;

        NodeWrap* sumarizeBlock() override;
        void onAttachBlock() override;
        void onDetachBlock() override;
        void buildHwComponent() override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
        /** debug method*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal* mdLogVal) override;
        /** override simulation*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;
    };

    /***
     *
     * cycle wait flowBlock
     *
     * */

    class FlowBlockCycleWait : public FlowBlockBase, public LoopStMacro{
    protected:
        /** result node wrap to summarize this block*/
        NodeWrap* _resultNodeWrap = nullptr;
        /** wait cycle node*/
        WaitCycleNode* _waitNode = nullptr;
        /** wait meta Data*/
        int cycle = -1;
        Operable* cnt = nullptr;

    public:

        explicit FlowBlockCycleWait(int cycleUsed);
        explicit FlowBlockCycleWait(Operable& opr);
        ~FlowBlockCycleWait() override;

        NodeWrap* sumarizeBlock() override;
        void onAttachBlock() override;
        void onDetachBlock() override;
        void buildHwComponent() override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
        /** debugger*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal* mdLogVal) override;
        /**override simulation*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;

    };

}

#endif //KATHRYN_WAIT_H
