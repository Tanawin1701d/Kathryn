//
// Created by tanawin on 15/1/2567.
//

#include "simController.h"
#include "modelCompile/proxyEventBase.h"

namespace kathryn{

    SimController::SimController():
    _limitCycle(0),
    _proxySimEvent(nullptr)
    {}

    void SimController::setProxySimEvent(ProxySimEventBase* proxySimEvent){
        _proxySimEvent = proxySimEvent;
    }


    void SimController::collectData() {}

    void SimController::start() {


        lock();
        while ( (!eventQ.isEmpty()) &&
                (eventQ.getNextEvent()->getCurCycle() <= _limitCycle)
        ){
            EventBase* nextEvent = eventQ.getNextEvent();

            /** simulate every cycle that event queue skip*/
            CYCLE remainCycle = (nextEvent->getCurCycle()) - _curCycle;
            for (int remCycle = 0; remCycle < remainCycle; remCycle++){
                _proxySimEvent->simStartCurCycle();
                _proxySimEvent->curCycleCollectData();
                _proxySimEvent->simStartNextCycle();
                _proxySimEvent->simExitCurCycle();
            }
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
                if (curEvent->needToDelete())
                    delete curEvent;
            }
            lock();

        }
        unlock();

        lock();
        for (; _curCycle < _limitCycle; _curCycle++){
            _proxySimEvent->simStartCurCycle();
            _proxySimEvent->curCycleCollectData();
            _proxySimEvent->simStartNextCycle();
            _proxySimEvent->simExitCurCycle();
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
        _proxySimEvent = nullptr; ///// proxy is create from sim interface
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

    ProxySimEventBase* SimController::getProxySimEventPtr(){
        return _proxySimEvent;
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