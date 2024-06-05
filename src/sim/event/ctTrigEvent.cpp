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
        /**notify that it is time to coputenext*/
        ///std::cout << "[TRIGGER]start trigger" << std::endl;
        startEvent.notify();
        ///std::cout << "[TRIGGER]wait for finish" << std::endl;
        /** wait for con sim is finish due to we dont want it race with hybrid model*/
        finishEvent.wait();
        ///std::cout << "[TRIGGER]finish trigger" << std::endl;
    }




}