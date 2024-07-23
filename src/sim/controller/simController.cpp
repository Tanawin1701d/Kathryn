//
// Created by tanawin on 15/1/2567.
//

#include "simController.h"

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
            std::cout << "curCycle is " << _curCycle << std::endl;
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

            CYCLE amtUserLimit     = (_amtLrLimUser == nullptr)? INT64_MAX: *_amtLrLimUser;
            CYCLE nextEventOccurAt = eventQ.isEmpty()          ? INT64_MAX: eventQ.getNextEvent()->getCurCycle();
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
            //////// long run cycle
            for (auto* curEvent: _curCycleEvents){
                int lrc = 0; //// long range counter
                if (curEvent->isLongRageSim()){
                    ///////////// intepret the cycle to run
                    lrc++;
                    CYCLE amtLimitByQueue  = nextEventOccurAt - curEvent->getCurCycle();
                    assert(amtLimitByQueue > 0 && amtUserLimit > 0);
                    curEvent->setLongRangeSim(std::min(amtUserLimit, amtLimitByQueue));
                    ///////////// start running
                    curEvent->simStartLongRunCycle();
                    for (int callBackIdx = 0; callBackIdx < curEvent->getCallBackAmt(); callBackIdx++){
                        int callBackNo = curEvent->getCallBackNo(callBackIdx);
                        assert(callBackNo < _mdTraceMap->size());
                        (*_mdTraceMap)[callBackNo].execCallBack();
                    }
                }
                assert(lrc <= 1); ///// we must have only or non for long rage sim
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

            if (stopMark){
                break;
            }

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

    void SimController::setLimitCycle(CYCLE lmtCycle){
        lock();
        _limitCycle = lmtCycle;
        unlock();
    }

    void SimController::reset(){
        lock();
        _limitCycle = 1;
        eventQ.reset();
        stopMark      = false;
        _mdTraceMap   = nullptr;
        _amtLrLimUser = nullptr;
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

    void SimController::setTriggerMap(std::vector<TraceEvent>* mdTraceMap){
        lock();
        assert(mdTraceMap != nullptr);
        _mdTraceMap = mdTraceMap;
        unlock();
    }

    void SimController::setLrLimUser(CYCLE* amtLrLimUser){
        assert(amtLrLimUser != nullptr);
        lock();
        _amtLrLimUser = amtLrLimUser;
        unlock();
    }

    void SimController::stopSim(){
        lock();
        stopMark = true;
        unlock();
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