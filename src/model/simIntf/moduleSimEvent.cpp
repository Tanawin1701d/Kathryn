//
// Created by tanawin on 21/1/2567.
//
#include "model/hwComponent/module/module.h"
#include "moduleSimEvent.h"

namespace kathryn{


    ModuleSimEvent::ModuleSimEvent(Module* startMd,
                                   Operable* resetWire,
                                   VcdWriter* writer):
    EventBase(0, SIM_MODEL_PRIO),
    _startModule(startMd),
    _resetWire(resetWire),
    _writer(writer)
    {
        assert(_startModule != nullptr);
        assert(_resetWire   != nullptr);

        /**prepare simulation*/
        _writer->addNewVar(VST_WIRE, "CLK", {0,1});
        _startModule->beforePrepareSim(_writer);
        _startModule->prepareSim();


        /** start Value*/
        addNewEvent(this);

    }


    void ModuleSimEvent::simStartCurCycle() {
        if (_curCycle == 0) {
            /** initiate reset Value*/
            auto resetSimEngine = _resetWire->castToRtlSimItf()->getSimEngine();
            ValRep resetVal = NumConverter::cvtStrToValRep(1, 1);
            resetSimEngine->setCurValSimStatus();
            resetSimEngine->getCurVal() = resetVal;
        }
        _startModule->simStartCurCycle();
        /***set reset wire */

    }

    void ModuleSimEvent::curCycleCollectData() {
        /*** CLK UP*/
        _writer->addNewTimeStamp(_curCycle * _clockIntv);
        auto upClk = NumConverter::cvtStrToValRep(1, 0b1);
        _writer->addNewValue("CLK", upClk);
        _startModule->curCycleCollectData();
        /*** CLK DOWN*/
        _writer->addNewTimeStamp(_curCycle * _clockIntv + (_clockIntv >> 1));
        auto downClk = NumConverter::cvtStrToValRep(1, 0b0);
        _writer->addNewValue("CLK", downClk);


    }

    void ModuleSimEvent::simExitCurCycle() {

        _startModule->simExitCurCycle();
        /** prepare next event by convert this class to be next event */
        assert(_resetWire != nullptr);
        /**change reset to 0*/
        _curCycle++;

        addNewEvent(this);
    }


}