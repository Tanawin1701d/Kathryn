//
// Created by tanawin on 21/1/2567.
//

#ifndef KATHRYN_MODULESIMEVENT_H
#define KATHRYN_MODULESIMEVENT_H

#include "sim/event/eventBase.h"

namespace kathryn{


    class ModuleSimEvent : public EventBase{
    private:
        Module*     _startModule = nullptr;
        Operable*   _resetWire   = nullptr;
        VcdWriter*  _writer      = nullptr;
        FlowColEle* _flowColEle  = nullptr;
        ull        _clockIntv    = 10;

    public:

        explicit ModuleSimEvent(Module*    startMd,
                                Operable*  resetWire, ///// active high
                                VcdWriter* writer,
                                FlowColEle* flowColEle
        );


        void simStartCurCycle()    override;
        void curCycleCollectData() override;
        void simStartNextCycle()   override;
        void simExitCurCycle()     override;

        bool needToDelete() override {return false;}
    };





}

#endif //KATHRYN_MODULESIMEVENT_H
