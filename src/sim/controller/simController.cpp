//
// Created by tanawin on 15/1/2567.
//

#include "simController.h"



namespace kathryn{

    SimController::SimController():
    _limitCycle(0)
    {}

    void SimController::collectData() {

    }

    void SimController::simStart() {

        //std::cout << eventQ.getNextEvent()->getCurCycle() << std::endl;
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
            lock();
            _curCycle = eventQ.getNextEvent()->getCurCycle();
            /************************************************************************/
            /** get event that occur in same cycle*/
            std::vector<EventBase*> curEvents = eventQ.getAndPopNextSameCycleEvent();
            unlock();
            /** simulate each event*/
            for (auto event : curEvents){
                event->simStartCurCycle();
            }
            /** collect each*/
            for (auto event : curEvents){
                event->curCycleCollectData();
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
        lock();
        eventQ.addEvent(event);
        unlock();
    }

    void SimController::saveData() {
        assert(false);
    }

    void SimController::reset(){
        lock();
        _limitCycle = 1;
        eventQ.reset();
        unlock();
    }

    CYCLE SimController::getCurCycle(){
        lock();
        CYCLE cpyCycle = _curCycle;
        unlock();
        return cpyCycle;
    }

    void SimController::lock(){
        _rsMtx.lock();
    }

    void SimController::unlock(){
        _rsMtx.unlock();
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
            simCtrl = new SimController();
        }
        return simCtrl;

    }

}