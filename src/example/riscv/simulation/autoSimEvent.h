//
// Created by tanawin on 15/6/2024.
//

#ifndef AUTOSIMEVENT_H
#define AUTOSIMEVENT_H

#include "example/riscv/core/core.h"
#include "model/simIntf/base/proxyEventBase.h"
#include "sim/event/eventBase.h"


namespace kathryn{
namespace riscv{

    class RiscvSimSort;
    class RiscvSimEvent: public EventBase{
    protected:
        RiscvSimSort*     _master = nullptr;
        int                testFinRegIdx = 31;
        ull                _curCycleCal  =  0;
    public:

    explicit RiscvSimEvent(RiscvSimSort* master);
    void simStartCurCycle()    override;

    void curCycleCollectData() override{}

    void simStartNextCycle()   override{}

    void simExitCurCycle()     override{}

    bool needToDelete()        override{return false;}

    virtual EventBase* genNextEvent() override;

    };

}
}

#endif //AUTOSIMEVENT_H
