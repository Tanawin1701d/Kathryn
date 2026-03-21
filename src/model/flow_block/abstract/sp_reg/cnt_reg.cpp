//
// Created by tanawin on 3/1/2567.
//

#include "cnt_reg.h"
#include "model/controller/controller.h"


namespace kathryn{

    /**
     *
     * counter register
     *
     * */

    /** constructor for specific cycle*/
    CounterReg::CounterReg(int max_cycle):
            CtrlFlowRegBase( cal_bit_used_in_counter(max_cycle),
         false,
         TYPE_COUNTER_REG,
         false
     ),
    _cntBitSz(cal_bit_used_in_counter(max_cycle)),
    _lastCycle(max_cycle),
    _idleVal(&make_opr_val("idle_val",_cntBitSz, 0))
     {

        /** TO FIX*/
        com_init();
        assert(max_cycle > 0);

     }

    UpdateEventBase* CounterReg::add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm){
        assert(depend_state != nullptr);

        auto* event = create_ue(activate_cond,
                               depend_state,
                               _idleVal,
                               Slice({0, _cntBitSz}),
                               DEFAULT_UE_PRI_INTERNAL_MAX,
                               cm
        );
        add_update_meta(event);
        return event;
    }


    void CounterReg::make_inc_event(Operable* up_count_event, CLOCK_MODE cm) {

        if (up_count_event != nullptr){
            assert(up_count_event->get_operable_slice().get_size() == 1);
        }
        auto* event = create_ue(
            nullptr,
            up_count_event,
            &((*this) + 1),
            Slice({0, _cntBitSz}),
            DEFAULT_UE_PRI_INTERNAL_MAX-1,
            cm
        );
        add_update_meta(event);
    }

    void CounterReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_CYCLE_WAIT_REG);
    }

    void CounterReg::make_un_set_state_event(CLOCK_MODE cm) {
        assert(false); //// it is unused
    }

    void CounterReg::make_user_rst_event(Operable* rst, CLOCK_MODE cm){
        /**reset event*/
        assert(rst != nullptr);
        auto* reset_event = create_ue(
           nullptr,
           rst,
           _idleVal,
           Slice({0, _cntBitSz}),
           DEFAULT_UE_PRI_INTERNAL_MIN,
            cm);
        add_update_meta(reset_event);

    }

    Operable* CounterReg::generate_end_expr() {
        return &(*this == (_lastCycle-1));
    }


}