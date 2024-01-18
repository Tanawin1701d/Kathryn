//
// Created by tanawin on 15/1/2567.
//

#include "controller.h"



namespace kathryn{

    SimController::SimController(CYCLE limitCycle):
    _limitCycle(limitCycle)
    {}

    void SimController::collectData() {

    }

    void SimController::simStart() {

        assert(_rstWire != nullptr);

        while ( (!eventQ.isEmpty()) &&
                (eventQ.getNextEvent()->getCurCycle() <= _limitCycle)
        ){
            /** exit reason*/
            if (eventQ.isEmpty()){
                std::cout << "[SIM EXIT] sim exit because eventQ is empty\n";
                break;
            }
            if (eventQ.getNextEvent()->getCurCycle() > _limitCycle){
                std::cout << "[SIM EXIT] sim exit because eventQ reaches limit cycle\n";
                break;
            }
            /************************************************************************/
            /** get event that occur in same cycle*/
            std::vector<EventBase*> curEvents = eventQ.getAndPopNextSameCycleEvent();
            /** simulate each event*/
            for (auto event : curEvents){
                event->simCurCycle();
            }
            /** delete each event*/
            for (auto event: curEvents){
                delete event;
            }
        }

    }

    void SimController::setResetSignal(Wire *resetWire) {
        assert(resetWire != nullptr);
        _rstWire = resetWire;
    }

    void SimController::addEvent(EventBase *event) {
        assert(event != nullptr);
        eventQ.addEvent(event);
    }

    void SimController::saveData() {
        assert(false);
    }

}