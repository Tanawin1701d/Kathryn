//
// Created by tanawin on 15/1/2567.
//

#ifndef KATHRYN_SIMCONTROLLER_H
#define KATHRYN_SIMCONTROLLER_H

#include<iostream>
#include<mutex>

#include "sim/event/eventBase.h"
#include "sim/event/eventQ.h"
#include "abstract/mainControlable.h"
#include "sim/modelSimEngine/base/traceEvent.h"

namespace kathryn{

    class ProxySimEventBase;
    class Wire;
    class SimController: public MainControlable{
    private:
        std::mutex              _rsMtx;
        CYCLE                   _limitCycle    =  1;
        CYCLE                   _curCycle      = -1;
        EventQ                  eventQ;
        bool                    stopMark        = false;
        ///// the trace idx is the idx of vector
        std::vector<TraceEvent>* _mdTraceMap   = nullptr;
        CYCLE*                   _amtLrLimUser = nullptr; //// amount cylce that user limit

        void collectData();

    public:

        explicit SimController();
        void  start() override;
        void  reset() override;
        void  clean() override;
        void  addEvent(EventBase* event);
        void  saveData();
        void  setLimitCycle(CYCLE lmtCycle);
        void  setTriggerMap(std::vector<TraceEvent>* mdTraceMap);
        void  setLrLimUser(CYCLE* amtLrLimUser);
        void  stopSim();
        CYCLE getCurCycle();
        void  lock();
        void  unlock();


    };

    SimController* getSimController();

}

#endif //KATHRYN_SIMCONTROLLER_H
