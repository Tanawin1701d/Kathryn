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
               (_flowBlockBase->getSimEngine() != nullptr);
    }

    bool SimProbe::isIdle(){
        return !isExecuting();
    }


    bool SimProbe::isWaiting(){
        mfAssert(false, "standard simprobe not support isWaiting()");
    }

    bool SimProbe::isExecuting(){
        mfAssert(checkSimEngineReady(), "cannot find sim engine for flow block to probe");
        FlowBaseSimEngine* fbse = _flowBlockBase->getSimEngine();
        return fbse->isBlockRunning();
    }

    
}