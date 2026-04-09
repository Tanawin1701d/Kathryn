//
// Created by tanawin on 2/1/2567.
//

#ifndef KATHRYN_WAIT_H
#define KATHRYN_WAIT_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/waitNode.h"
#include "model/flowBlock/abstract/loopStMacro.h"

#define scWait( cond) for(auto kathryn_block = new FlowBlockCondWait(cond)  ; kathryn_block->do_pre_post_function(); kathryn_block->step()){};
#define syWait(cycle) for(auto kathryn_block = new FlowBlockCycleWait(cycle); kathryn_block->do_pre_post_function(); kathryn_block->step()){};

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

        NodeWrap* sumarize_block() override;
        void on_attach_block() override;
        void on_detach_block() override;
        void build_hw_component() override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;
        /** debug method*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal* mdLogVal) override;
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
        explicit FlowBlockCycleWait(Operable& opr1);
        ~FlowBlockCycleWait() override;

        NodeWrap* sumarize_block() override;
        void on_attach_block() override;
        void on_detach_block() override;
        void build_hw_component() override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;
        /** debugger*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal* mdLogVal) override;
    };

}

#endif //KATHRYN_WAIT_H
