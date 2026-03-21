//
// Created by tanawin on 11/10/25.
//

#include "flow_block_pip_prober.h"

#include "cassert"

#include "model/flow_block/pipeline/pipe.h"
#include "model/flow_block/pipeline/zync.h"


namespace kathryn{


    /**
     * PIPE BLOCK
     */
    void PipSimProbe::init(FlowBlockPipeBase* flow_block_pip){
        assert(flow_block_pip != nullptr);
        flow_block_pip_base = flow_block_pip;
        SimProbe::init(flow_block_pip);
    }

    bool PipSimProbe::is_idle(){
        return !is_waiting() && !is_executing();
    }

    bool PipSimProbe::is_waiting(){
        check_sim_engine_ready();
        StateNode* wait_node = flow_block_pip_base->get_wait_node_ptr();
        StateReg&  state_reg = *wait_node->_state_reg;
        bool is_waiting  = static_cast<bool>((ull)state_reg);
        return is_waiting;

    }

    bool PipSimProbe::is_executing(){
        check_sim_engine_ready();
        FlowBlockBase* implic_fb = flow_block_pip_base->get_implicit_flow_block_ptr();
        assert(implic_fb != nullptr);
        return implic_fb->get_sim_engine_ptr()->is_block_running();
    }


    /**
     * ZYNC BLOCK
     **/

    bool ZyncSimProb::get_prep_send_node_status(){
        StateNode* pre_send_node = flow_block_zync_base->get_pre_send_node_ptr();
        StateReg&  state_reg = *pre_send_node->_state_reg;
        bool is_prep_send = static_cast<bool>((ull)state_reg);
        return is_prep_send;
    }

    bool ZyncSimProb::get_zync_ready_status(){
        SyncMeta& sync_meta = flow_block_zync_base->get_sync_meta();
        return ((ull)sync_meta._syncMatched);
    }


    void ZyncSimProb::init(FlowBlockZyncBase* flow_block_zync){
        assert(flow_block_zync != nullptr);
        flow_block_zync_base = flow_block_zync;
        SimProbe::init(flow_block_zync);
    }

    bool ZyncSimProb::is_idle(){
        return !is_waiting() && !is_executing();
    }

    bool ZyncSimProb::is_waiting(){
        check_sim_engine_ready();
        bool is_prep_send = get_prep_send_node_status();
        bool is_zync_ready = get_zync_ready_status();
        return is_prep_send && (!is_zync_ready);
    }

    bool ZyncSimProb::is_executing(){
        check_sim_engine_ready();
        bool is_prep_send = get_prep_send_node_status();
        bool is_zync_ready = get_zync_ready_status();
        return is_prep_send && is_zync_ready;
    }

}
