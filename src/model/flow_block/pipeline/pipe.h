//
// Created by tanawin on 1/3/2025.
//

#ifndef SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H
#define SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H


#include "sync_meta.h"
#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/loop_st_macro.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"
#include "sim/model_sim_engine/flow_block/pipeline/flow_block_pip_prober.h"


#define pip(zync_meta)       for(auto kathryn_block = new FlowBlockPipeBase (zync_meta); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define auto_sync kathryn_block->set_auto_activate_pipe();


namespace kathryn{


    const char PIPE_UNNAME[] = "PIPE_UNNAMED";


    class FlowBlockPipeBase: public FlowBlockBase,  public LoopStMacro{
    protected:
        SyncMeta& _syncMata;
        const std::string _pipeName = PIPE_UNNAME;
        ///////////// meta Data
        bool auto_activate_pipe     = false;
        bool is_get_flow_block_yet    = false;
        ///////////// node
        PseudoNode* ent_node       = nullptr; //// entrance node
        StateNode*  wait_node      = nullptr;
        DummyNode*  exit_dummy     = nullptr; //// pipeline is perpeptual engine
        //////////// block implicit flow block
        FlowBlockBase* implicit_flow_block = nullptr;
        NodeWrap*      sub_block_node_wrap  = nullptr;
        NodeWrap*      result_node_wrap    = nullptr;

    public:
        explicit FlowBlockPipeBase(SyncMeta& sync_meta); ///// perpeptual loop indicate that it will loop when subblock is finish
        ~FlowBlockPipeBase() override;
        /** for controller add the local element to this sub block*/
        void add_element_in_flow_block (Node* node) override;
        void add_sub_flow_block       (FlowBlockBase* sub_block) override;
        void add_con_flow_block       (FlowBlockBase* con_block) override;
        void add_abandon_flow_block   (FlowBlockBase* abandon_block) override;
        NodeWrap* sumarize_block    () override;
        /**set activate bias usually used in init Pipe */
        void      assign_ready_signal ();

        std::string    get_pipe_name()          const{return _pipeName;}
        StateNode*get_wait_node_ptr()          const{assert(wait_node != nullptr); return wait_node;}
        FlowBlockBase*get_implicit_flow_block_ptr() const{return implicit_flow_block;}
        /** auto activate pipe*/
        void      set_auto_activate_pipe(){auto_activate_pipe = true;}
        bool      is_auto_activate_pipe() const{return auto_activate_pipe;}
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


#endif //SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H
