//
// Created by tanawin on 21/1/2567.
//
#include "model/hwComponent/module/module.h"
#include "moduleSimEvent.h"

namespace kathryn{


    ModuleSimEvent::ModuleSimEvent(Module* startMd,
                                   Operable* resetWire,
                                   StartNode* startNode):
    EventBase(0, SIM_MODEL_PRIO),
    _startModule(startMd),
    _resetWire(rstWire),
    _startNode(startNode),
    _curCycle(0)
    {
        assert(_startModule != nullptr);
        assert(_resetWire   != nullptr);

        auto resetSimEngine = _resetWire->castToRtlSimItf()->getSimEngine();
        ValRep resetVal = NumConverter::cvtStrToValRep(1, 1);
        resetSimEngine->getCurVal() = resetVal;
        addNewEvent(this);

    }


    void ModuleSimEvent::simStartCurCycle() {
        _startModule->simStartCurCycle();
    }

    void ModuleSimEvent::curCycleCollectData() {
        _startModule->curCycleCollectData();
    }

    void ModuleSimEvent::simExitCurCycle() {

        _startModule->simExitCurCycle();
        /** process next event*/
        assert(_resetWire != nullptr);
        auto resetSimEngine = _resetWire->castToRtlSimItf()->getSimEngine();
        ValRep resetVal = NumConverter::cvtStrToValRep(1,0);
        resetSimEngine->getCurVal() = resetVal;
        _curCycle++;

        addNewEvent(this);
    }


}