//
// Created by tanawin on 21/1/2567.
//
#include "model/hwComponent/module/module.h"
#include "moduleSimEvent.h"

namespace kathryn{


    ModuleSimEvent::ModuleSimEvent(Module* startMd,
                                   Operable* resetWire,
                                   VcdWriter* writer,
                                   FlowColEle* flowColEle
                                   ):
    EventBase(0, SIM_MODEL_PRIO),
    _startModule(startMd),
    _resetWire(resetWire),
    _writer(writer),
    _flowColEle(flowColEle)
    {
        assert(_startModule != nullptr);
        assert(_resetWire   != nullptr);
        assert(_flowColEle  != nullptr);

        /**prepare simulation*/
        _writer->addNewVar(VST_WIRE, "CLK", {0,1});
        _startModule->beforePrepareSim(_writer, _flowColEle);
        _startModule->prepareSim();


        /** start Value*/
        addNewEvent(this);

    }


    void ModuleSimEvent::simStartCurCycle() {
        if (_curCycle == 0) {
            /** initiate reset Value*/
            ValRep resetVal = NumConverter::cvtStrToValRep(1, 1);
            _resetWire->getRtlValItf()->setCurValSimStatus();
            _resetWire->getRtlValItf()->getCurVal() = resetVal;
        }
        ////std::cout << "--------------------------------------------------------" <<std::endl;
        _startModule->simStartCurCycle();
        _startModule->simStartNextCycle();
        ////std::cout << "--------------------------------------------------------" <<std::endl;
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