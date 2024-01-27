//
// Created by tanawin on 21/1/2567.
//
#include "model/hwComponent/module/module.h"
#include "moduleSimEvent.h"

namespace kathryn{


    ModuleSimEvent::ModuleSimEvent(Module* startMd,
                                   Operable* resetWire,
                                   StartNode* startNode,
                                   VcdWriter* writer):
    EventBase(0, SIM_MODEL_PRIO),
    _startModule(startMd),
    _resetWire(rstWire),
    _startNode(startNode),
    _writer(writer)
    {
        assert(_startModule != nullptr);
        assert(_resetWire   != nullptr);

        /**prepare simulation*/
        _startModule->beforePrepareSim(_writer);
        _startModule->prepareSim();

        /** initiate reset Value*/
        auto resetSimEngine = _resetWire->castToRtlSimItf()->getSimEngine();
        ValRep resetVal = NumConverter::cvtStrToValRep(1, 1);
        resetSimEngine->getCurVal() = resetVal;
        /** start Value*/
        addNewEvent(this);

    }


    void ModuleSimEvent::simStartCurCycle() {
        _startModule->simStartCurCycle();
    }

    void ModuleSimEvent::curCycleCollectData() {
        /*** CLK UP*/
        _writer->addNewTimeStamp(_curCycle * _clockIntv);
        _writer->addNewVar(VST_WIRE, "CLK", {0,1});
        _startModule->curCycleCollectData();
        /*** CLK DOWN*/
        _writer->addNewTimeStamp(_curCycle * _clockIntv + (_clockIntv >> 1));
        _writer->addNewVar(VST_WIRE, "CLK", {0, 1});

    }

    void ModuleSimEvent::simExitCurCycle() {

        _startModule->simExitCurCycle();
        /** prepare next event by convert this class to be next event */
        assert(_resetWire != nullptr);
        /**change reset to 0*/
        auto resetSimEngine = _resetWire->castToRtlSimItf()->getSimEngine();
        ValRep resetVal = NumConverter::cvtStrToValRep(1,0);
        resetSimEngine->getCurVal() = resetVal;
        _curCycle++;

        addNewEvent(this);
    }


}