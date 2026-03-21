//
// Created by tanawin on 4/12/2566.
//

#ifndef KATHRYN_PAR_H
#define KATHRYN_PAR_H


#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

#define par    for(auto kathryn_block = new FlowBlockParAuto();   kathryn_block->do_pre_post_function(); kathryn_block->step())
#define par_man for(auto kathryn_block = new FlowBlockParNoSync(); kathryn_block->do_pre_post_function(); kathryn_block->step())


namespace kathryn {



    class FlowBlockPar : public FlowBlockBase, public LoopStMacro{
    protected:

        /** metas element*/
        NodeWrap* result_node_wrap = nullptr;

        /** check that subblock or basic node contain control flow operation*/
        std::vector<NodeWrap*> node_wrap_of_sub_block;
        int cycle_used = -1;
        /** master_joiner_block*/
        //////// incase there is manual specify to make it manual sync block
        FlowBlockBase* master_join_flow_block = nullptr;

        StateNode*  basic_st_node    = nullptr;
        SynNode*    syn_node        = nullptr;
        PseudoNode* pseudo_exit_node = nullptr;
        /** pseudo exit node will be null if this class
         * did not synthesize the node
         * */

    public:

        explicit FlowBlockPar(FLOW_BLOCK_TYPE fb_type);
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
        virtual void build_sync_node() = 0;
        virtual void assign_exit_to_rnw() = 0; /// Rnw <= result node wrap
        virtual void assign_cycle_used_to_rnw();
        virtual void assign_force_exit_to_rnw();

        std::string get_md_describe() override;
        void        add_md_log(MdLogVal *md_log_val) override;

    };

    /** this parallel block auto build synchronizer*/
    class FlowBlockParAuto: public FlowBlockPar{
    public:
        explicit FlowBlockParAuto(): FlowBlockPar(PARALLEL_AUTO_SYNC){}
        void build_sync_node() override;
        void assign_exit_to_rnw() override; /// Rnw <= result node wrap
    };
    /** this parallel block no sync at all*/
    class FlowBlockParNoSync: public FlowBlockPar{
    public:
        explicit FlowBlockParNoSync(): FlowBlockPar(PARALLEL_NO_SYN){}
        void build_sync_node() override{/** we don't build sync Node*/};
        void assign_exit_to_rnw() override;
    };



}



#endif //KATHRYN_PAR_H
