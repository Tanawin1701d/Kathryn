//
// Created by tanawin on 15/1/2567.
//

#ifndef KATHRYN_SIMCONTROLLER_H
#define KATHRYN_SIMCONTROLLER_H

#include<iostream>
#include<mutex>

#include "sim/event/eventBase.h"
#include "sim/event/eventQ.h"

namespace kathryn{

    class Wire;
    class SimController{

    private:
        std::mutex _rsMtx;
        CYCLE _limitCycle = 1;
        CYCLE _curCycle   = -1;
        EventQ eventQ;

        void collectData();

    public:

        explicit SimController();
        void  simStart();
        void  addEvent(EventBase* event);
        void  saveData();
        void  setLimitCycle(CYCLE lmtCycle){_limitCycle = lmtCycle;}
        void  reset();
        CYCLE getCurCycle();
        void  lock();
        void  unlock();

    };

    SimController* getSimController();

}

#endif //KATHRYN_SIMCONTROLLER_H
