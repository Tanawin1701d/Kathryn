//
// Created by tanawin on 11/10/25.
//

#include "flowBlockProber.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"



namespace kathryn{


    void SimProbe::init(FlowBlockBase* flowBlockBase){
        _flowBlockBase = flowBlockBase;
    }

    bool SimProbe::checkSimEngineReady(){
        return (_flowBlockBase != nullptr) &&
               (_flowBlockBase->get_sim_engine() != nullptr);
    }

    bool SimProbe::isIdle(){
        return !isExecuting();
    }


    bool SimProbe::isWaiting(){
        mf_assert(false, "standard simprobe not support isWaiting()");
        return false;
    }

    bool SimProbe::isExecuting(){
        mf_assert(checkSimEngineReady(), "cannot find sim engine for flow block to probe");
        FlowBaseSimEngine* fbse = _flowBlockBase->get_sim_engine();
        return fbse->isBlockRunning();
    }

    
}