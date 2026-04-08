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
    _cnt_bit_sz(calBitUsedInCounter(maxCycle)),
    _last_cycle(maxCycle),
    _idle_val(&makeOprVal("idleVal",_cnt_bit_sz, 0))
     {

        /** TO FIX*/
        com_init();
        assert(maxCycle > 0);

     }

    UpdateEventBase* CounterReg::add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        assert(dependState != nullptr);

        auto* event = createUE(activateCond,
                               dependState,
                               _idle_val,
                               Slice({0, _cnt_bit_sz}),
                               DEFAULT_UE_PRI_INTERNAL_MAX,
                               cm
        );
        addUpdateMeta(event);
        return event;
    }


    void CounterReg::make_inc_event(Operable* upCountEvent, CLOCK_MODE cm) {

        if (upCountEvent != nullptr){
            assert(upCountEvent->getOperableSlice().getSize() == 1);
        }
        auto* event = createUE(
            nullptr,
            upCountEvent,
            &((*this) + 1),
            Slice({0, _cnt_bit_sz}),
            DEFAULT_UE_PRI_INTERNAL_MAX-1,
            cm
        );
        addUpdateMeta(event);
    }

    void CounterReg::com_init() {
        _ctrl->on_sp_reg_init(this, SP_CYCLE_WAIT_REG);
    }

    void CounterReg::make_un_set_state_event(CLOCK_MODE cm) {
        assert(false); //// it is unused
    }

    void CounterReg::makeUserRstEvent(Operable* rst, CLOCK_MODE cm){
        /**reset event*/
        assert(rst != nullptr);
        auto* resetEvent = createUE(
           nullptr,
           rst,
           _idle_val,
           Slice({0, _cnt_bit_sz}),
           DEFAULT_UE_PRI_INTERNAL_MIN,
            cm);
        addUpdateMeta(resetEvent);

    }

    Operable* CounterReg::generateEndExpr() {
        return &(*this == (_last_cycle-1));
    }


}