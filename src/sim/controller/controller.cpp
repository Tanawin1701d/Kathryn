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
                event->simStartCurCycle();
            }
            /** exit each event*/
            for (auto event : curEvents){
                event->simExitCurCycle();
            }
            /** delete each event*/
            for (auto event: curEvents){
                if (event->needToDelete())
                    delete event;
            }
        }

    }

    void SimController::addEvent(EventBase *event) {
        assert(event != nullptr);
        eventQ.addEvent(event);
    }

    void SimController::saveData() {
        assert(false);
    }


    /***
     *
     *
     * get sim controller (lazy initialization)
     *
     * **/

    SimController* simCtrl = nullptr;

    SimController* getSimController(){

        if (simCtrl == nullptr){
            simCtrl = new SimController(100000);
        }
        return simCtrl;

    }

}