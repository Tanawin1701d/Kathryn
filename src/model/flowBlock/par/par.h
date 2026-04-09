//
// Created by tanawin on 4/12/2566.
//

#ifndef KATHRYN_PAR_H
#define KATHRYN_PAR_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define par    for(auto kathryn_block = new FlowBlockParAuto();   kathryn_block->do_pre_post_function(); kathryn_block->step())
#define parMan for(auto kathryn_block = new FlowBlockParNoSync(); kathryn_block->do_pre_post_function(); kathryn_block->step())


namespace kathryn {



    class FlowBlockPar : public FlowBlockBase, public LoopStMacro{
    protected:

        /** metas element*/
        NodeWrap* resultNodeWrap = nullptr;

        /** check that subblock or basic node contain control flow operation*/
        std::vector<NodeWrap*> nodeWrapOfSubBlock;
        int cycleUsed = -1;
        /** masterJoinerBlock*/
        //////// incase there is manual specify to make it manual sync block
        FlowBlockBase* masterJoinFlowBlock = nullptr;

        StateNode*  basicStNode    = nullptr;
        SynNode*    synNode        = nullptr;
        PseudoNode* pseudoExitNode = nullptr;
        /** pseudo exit node will be null if this class
         * did not synthesize the node
         * */

    public:

        explicit FlowBlockPar(FLOW_BLOCK_TYPE fbType);
        ~FlowBlockPar();
        /** override flow block base*/
        NodeWrap* sumarize_block() override; /// to interact from parrent block call

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;

        /** for module to build hardware component*/
        void build_hw_component() override;

        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        /** build sync Node according to parallel policies*/
        virtual void buildSyncNode() = 0;
        virtual void assignExitToRnw() = 0; /// Rnw <= result node wrap
        virtual void assignCycleUsedToRnw();
        virtual void assignForceExitToRnw();

        std::string get_md_describe() override;
        void        add_md_log(MdLogVal *mdLogVal) override;

    };

    /** this parallel block auto build synchronizer*/
    class FlowBlockParAuto: public FlowBlockPar{
    public:
        explicit FlowBlockParAuto(): FlowBlockPar(PARALLEL_AUTO_SYNC){}
        void buildSyncNode() override;
        void assignExitToRnw() override; /// Rnw <= result node wrap
    };
    /** this parallel block no sync at all*/
    class FlowBlockParNoSync: public FlowBlockPar{
    public:
        explicit FlowBlockParNoSync(): FlowBlockPar(PARALLEL_NO_SYN){}
        void buildSyncNode() override{/** we don't build sync Node*/};
        void assignExitToRnw() override;
    };



}



#endif //KATHRYN_PAR_H
