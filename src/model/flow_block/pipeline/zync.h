//
// Created by tanawin on 13/9/25.
//

#ifndef SRC_MODEL_FLOW_BLOCK_PIPELINE_ZYNC_H
#define SRC_MODEL_FLOW_BLOCK_PIPELINE_ZYNC_H

#include "sync_meta.h"
#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

#define zync(zync_meta)        for(auto kathryn_block = new FlowBlockZyncBase(zync_meta, nullptr); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define zyncc(zync_meta, cond) for(auto kathryn_block = new FlowBlockZyncBase(zync_meta, &cond  ); kathryn_block->do_pre_post_function(); kathryn_block->step())

//#define zync_con(cond)        for(auto kathryn_block = new FlowBlockZyncBase(&cond);             kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockZyncBase: public FlowBlockBase, public LoopStMacro{
    protected:
        const std::string _zyncName = "ZYNC_UNNAMED";
        SyncMeta& _syncMeta;
        Operable* _acceptCond = nullptr; ///// this condition must be true when the system is in prep_send_node,
                                            ///// if the pre_send will not wait and not activate further layer
        ////// meta data
        bool auto_activate_pipe = false;


        ////// node
        StateNode*  prep_send_node = nullptr;
        PseudoNode* exit_node     = nullptr;
        /////// expresion
        ////// node wrap for summarize
        NodeWrap*      result_node_wrap    = nullptr;

    public:
        FlowBlockZyncBase(SyncMeta& sync_meta, Operable* accept_cond = nullptr);
        ~FlowBlockZyncBase() override;
        /** manage the system */
        void assign_ready_signal();
        StateNode*get_pre_send_node_ptr() { assert(prep_send_node != nullptr); return prep_send_node;}
        SyncMeta& get_sync_meta() const { return _syncMeta;}

        /** for controller add the local element to this sub block*/
        void add_sub_flow_block       (FlowBlockBase* sub_block) override;
        void add_con_flow_block       (FlowBlockBase* con_block) override;
        NodeWrap* sumarize_block    () override;
        /** auto activate pipe*/
        void set_auto_activate_pipe(){auto_activate_pipe = true;}
        bool is_auto_activate_pipe() const {return auto_activate_pipe;}

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_master() override;
        void build_hw_component() override;
        /** get describe*/
        void add_md_log(MdLogVal* md_log_val) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;


    };

}

#endif //SRC_MODEL_FLOW_BLOCK_PIPELINE_ZYNC_H