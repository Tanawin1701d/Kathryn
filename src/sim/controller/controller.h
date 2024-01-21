//
// Created by tanawin on 15/1/2567.
//

#ifndef KATHRYN_CONTROLLER_H
#define KATHRYN_CONTROLLER_H


#include "sim/event/eventBase.h"
#include "sim/event/eventQ.h"
#include "model/hwComponent/wire/wire.h"

namespace kathryn{

    class SimController{

    private:

        CYCLE nextCycle = 1;
        CYCLE _limitCycle = 1;
        Wire* _rstWire = nullptr;
        EventQ eventQ;

        void collectData();

    public:

        explicit SimController(CYCLE limitCycle);
        void simStart();
        void addEvent(EventBase* event);
        void saveData();

    };

    SimController* getSimController();

}

#endif //KATHRYN_CONTROLLER_H
