//
// Created by tanawin on 15/1/2567.
//

#include "simController.h"
#include "model/simIntf/base/proxyEventBase.h"

namespace kathryn{

    SimController::SimController():
    _limitCycle(0){}

    void SimController::collectData() {}

    void SimController::start() {


        lock();
        while ( (!eventQ.isEmpty()) &&
                (eventQ.getNextEvent()->getCurCycle() <= _limitCycle)
        ){

            EventBase* nextEvent = eventQ.getNextEvent();
            /**we are sure that nextEvent is valid due to while loop at the top*/
            assert(_curCycle != nextEvent->getCurCycle()); //// check therer is no same cycle used
            _curCycle = nextEvent->getCurCycle();
            std::vector<EventBase*> _curCycleEvents;
            /**
             *
             * start cur cycle which allows same cycle queue adding
             *
             * */

            while ((!eventQ.isEmpty()) && (eventQ.getNextEvent()->getCurCycle() == _curCycle)){
                _curCycleEvents.push_back(eventQ.getNextEvent());
                eventQ.popEvent();
            }
            /**
             * all event is simulated. For now, This cycle is stable.
             * */
            unlock();
            for (auto* curEvent: _curCycleEvents){
                curEvent->simStartCurCycle();
            }
            for (auto* curEvent: _curCycleEvents){
                curEvent->curCycleCollectData();
            }
            for (auto* curEvent: _curCycleEvents){
                curEvent->simStartNextCycle();
            }
            for (auto* curEvent: _curCycleEvents){
                curEvent->simExitCurCycle();
            }
            for (auto curEvent: _curCycleEvents){
                EventBase* afterEvent = curEvent->genNextEvent();
                if (afterEvent != nullptr){
                    addEvent(afterEvent);
                }
                if (curEvent->needToDelete()){
                    delete curEvent;
                }
            }
            lock();

        }
        unlock();

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

    void SimController::clean(){
        reset();
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