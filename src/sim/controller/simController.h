//
// Created by tanawin on 15/1/2567.
//

#ifndef KATHRYN_SIMCONTROLLER_H
#define KATHRYN_SIMCONTROLLER_H

#include<iostream>

#include "sim/event/eventBase.h"
#include "sim/event/eventQ.h"

namespace kathryn{

    class Wire;
    class SimController{

    private:

        CYCLE nextCycle = 1;
        CYCLE _limitCycle = 1;
        Wire* _rstWire = nullptr;
        EventQ eventQ;

        void collectData();

    public:

        explicit SimController();
        void simStart();
        void addEvent(EventBase* event);
        void saveData();
        void setLimitCycle(CYCLE lmtCycle){_limitCycle = lmtCycle;}

    };

    SimController* getSimController();

}

#endif //KATHRYN_SIMCONTROLLER_H
