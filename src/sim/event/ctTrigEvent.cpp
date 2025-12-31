//
// Created by tanawin on 29/2/2567.
//

#include "ctTrigEvent.h"
#include "sim/interface/simInterface.h"



namespace kathryn{

    ConcreteTriggerEvent::ConcreteTriggerEvent(CYCLE targetCycle,
                                               SimInterface* simInterfaceMaster,
                                               std::function<bool()> conditionTrigger,
                                               int prority):
    EventBase(targetCycle,
              prority,
              false),
              _simInterfaceMaster(simInterfaceMaster),
              _conditionTrigger(conditionTrigger){
        getSimController()->addEvent(this);
    }

    void ConcreteTriggerEvent::simStartCurCycleNeg() {
        startSimCurEvent.notify(getCurCycle()); //// notify the run aside thread
        finishSimCurEvent.wait(getCurCycle());  //// wait for the run aside thread to finish
    }

    void ConcreteTriggerEvent::simStartNextCyclePos(){
        startEndCycleEvent.notify(getCurCycle()); //// notify the run aside thread
        finishEndCycleEvent.wait(getCurCycle());  //// wait for the run aside thread to finish
    }



    EventBase* ConcreteTriggerEvent::genNextEvent(){
        if (stop){return nullptr;}
        _targetCycle = nextCycle;
        return this;
    }




}