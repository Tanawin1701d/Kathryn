//
// Created by tanawin on 11/10/25.
//

#include "flow_block_prober.h"
#include "model/flow_block/abstract/flow_block__base.h"



namespace kathryn{


    void SimProbe::init(FlowBlockBase* flow_block_base){
        _flowBlockBase = flow_block_base;
    }

    bool SimProbe::check_sim_engine_ready(){
        return (_flowBlockBase != nullptr) &&
               (_flowBlockBase->get_sim_engine_ptr() != nullptr);
    }

    bool SimProbe::is_idle(){
        return !is_executing();
    }


    bool SimProbe::is_waiting(){
        mf_assert(false, "standard simprobe not support is_waiting()");
        return false;
    }

    bool SimProbe::is_executing(){
        mf_assert(check_sim_engine_ready(), "cannot find sim engine for flow block to probe");
        FlowBaseSimEngine* fbse = _flowBlockBase->get_sim_engine_ptr();
        return fbse->is_block_running();
    }

    
}