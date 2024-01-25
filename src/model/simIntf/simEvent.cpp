//
// Created by tanawin on 21/1/2567.
//

#include "simEvent.h"
#include "util/numberic/numConvert.h"

namespace kathryn{


    ModuleSimEvent::ModuleSimEvent(Module* startMd,Operable* resetWire):
    EventBase(0),
    _startModule(startMd),
    _resetWire(rstWire),
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

    void ModuleSimEvent::simExitCurCycle() {

        _startModule->simExitCurCycle();

        assert(_resetWire != nullptr);
        auto resetSimEngine = _resetWire->castToRtlSimItf()->getSimEngine();
        ValRep resetVal = NumConverter::cvtStrToValRep(1,0);
        resetSimEngine->getCurVal() = resetVal;
        _curCycle++;

        addNewEvent(this);
    }
}