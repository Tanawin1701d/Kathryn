//
// Created by tanawin on 21/1/2567.
//

#ifndef KATHRYN_SIMEVENT_H
#define KATHRYN_SIMEVENT_H


#include "model/hwComponent/module/module.h"
#include "sim/event/eventBase.h"

namespace kathryn{


    class ModuleSimEvent : public EventBase{
    private:
        Module*   _startModule = nullptr;
        Operable* _resetWire   = nullptr;
        CYCLE     _curCycle    = 0;

    public:

        explicit ModuleSimEvent(Module*   startMd,
                                Operable* resetWire ///// active high
        );


        void simStartCurCycle() override;
        void simExitCurCycle() override;

        bool needToDelete() override {return true;}
    };





}

#endif //KATHRYN_SIMEVENT_H
