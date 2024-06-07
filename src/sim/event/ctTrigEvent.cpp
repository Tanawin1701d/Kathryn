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
              prority),
              _simInterfaceMaster(simInterfaceMaster),
              _conditionTrigger(conditionTrigger){
        getSimController()->addEvent(this);
    }

    void ConcreteTriggerEvent::simStartCurCycle() {
        startSimCurEvent.notify(getCurCycle());
        finishSimCurEvent.wait(getCurCycle());
    }

    void ConcreteTriggerEvent::simStartNextCycle(){
        startEndCycleEvent.notify(getCurCycle());
        finishEndCycleEvent.wait(getCurCycle());
    }



    EventBase* ConcreteTriggerEvent::genNextEvent(){
        if (stop){return nullptr;}
        _targetCycle = nextCycle;
        return this;
    }




}