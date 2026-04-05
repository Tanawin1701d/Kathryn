//
// Created by tanawin on 3/1/2567.
//

#include "cntReg.h"
#include "model/controller/controller.h"


namespace kathryn{

    /**
     *
     * counter register
     *
     * */

    /** constructor for specific cycle*/
    CounterReg::CounterReg(int maxCycle):
            CtrlFlowRegBase( calBitUsedInCounter(maxCycle),
         false,
         TYPE_COUNTER_REG,
         false
     ),
    _cntBitSz(calBitUsedInCounter(maxCycle)),
    _lastCycle(maxCycle),
    _idleVal(&makeOprVal("idleVal",_cntBitSz, 0))
     {

        /** TO FIX*/
        comInit();
        assert(maxCycle > 0);

     }

    UpdateEventBase* CounterReg::addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        assert(dependState != nullptr);

        auto* event = createUe(activateCond,
                               dependState,
                               _idleVal,
                               Slice({0, _cntBitSz}),
                               DEFAULT_UE_PRI_INTERNAL_MAX,
                               cm
        );
        addUpdateMeta(event);
        return event;
    }


    void CounterReg::makeIncEvent(Operable* upCountEvent, CLOCK_MODE cm) {

        if (upCountEvent != nullptr){
            assert(upCountEvent->getOperableSlice().getSize() == 1);
        }
        auto* event = createUe(
            nullptr,
            upCountEvent,
            &((*this) + 1),
            Slice({0, _cntBitSz}),
            DEFAULT_UE_PRI_INTERNAL_MAX-1,
            cm
        );
        addUpdateMeta(event);
    }

    void CounterReg::comInit() {
        ctrl->onSpRegInit(this, SP_CYCLE_WAIT_REG);
    }

    void CounterReg::makeUnSetStateEvent(CLOCK_MODE cm) {
        assert(false); //// it is unused
    }

    void CounterReg::makeUserRstEvent(Operable* rst, CLOCK_MODE cm){
        /**reset event*/
        assert(rst != nullptr);
        auto* resetEvent = createUe(
           nullptr,
           rst,
           _idleVal,
           Slice({0, _cntBitSz}),
           DEFAULT_UE_PRI_INTERNAL_MIN,
            cm);
        addUpdateMeta(resetEvent);

    }

    Operable* CounterReg::generateEndExpr() {
        return &(*this == (_lastCycle-1));
    }


}