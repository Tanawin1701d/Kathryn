//
// Created by tanawin on 3/1/2567.
//

#include "waitReg.h"
#include "model/controller/controller.h"


namespace kathryn{


    /**
     *
     * conditional wait state register
     *
     * */


    CondWaitStateReg::CondWaitStateReg(Operable *condOpr) :
    CtrlFlowRegBase(1,
                    false,
                    TYPE_COND_WAIT_STATE_REG,
                    false
    ),
    _cond_opr(condOpr)
    {
        /** init comunication to system*/
        assert(condOpr != nullptr);
        com_init();
        /** generate update event for exit event*/

    }

    void CondWaitStateReg::com_init() {
        _ctrl->on_sp_reg_init(this, SP_COND_WAIT_REG);
    }

    UpdateEventBase* CondWaitStateReg::add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        assert(dependState != nullptr);
        auto* event = create_ue(activateCond,
                               dependState,
                               &_up_state,
                               Slice({0, 1}),
                               DEFAULT_UE_PRI_INTERNAL_MAX,
                               cm
        );
        addUpdateMeta(event);
        return event;
    }

    void CondWaitStateReg::make_un_set_state_event(CLOCK_MODE cm) {
        auto* resetEvent = create_ue(_cond_opr,
                                    &((*this) == _up_state),
                                    &_down_state,
                                    Slice({0,1}),
                                    DEFAULT_UE_PRI_INTERNAL_MIN,
                                    cm);
        addUpdateMeta(resetEvent);
    }

    void CondWaitStateReg::make_user_rst_event(Operable* rst, CLOCK_MODE cm){
        assert(rst != nullptr);
        auto* resetEvent = create_ue(nullptr,
                                    rst,
                                    &_down_state,
                                    Slice({0,1}),
                                    DEFAULT_UE_PRI_INTERNAL_MIN,
                                    cm
                                    );
        addUpdateMeta(resetEvent);
    }

    Operable* CondWaitStateReg::generate_end_expr() {
        return &((*_cond_opr) & ((*this) == _up_state));
    }

    /**
     *
     * cycle count wait state register
     *
     * */

    /** constructor for specific cycle*/
    CycleWaitStateReg::CycleWaitStateReg(int waitCycle):
            CtrlFlowRegBase( cal_bit_used(waitCycle) + 1,
         false,
         TYPE_CYCLE_WAIT_STATE_REG,
         false
     ),
            _wait_cycle(waitCycle),
            _cnt_bit_sz     (cal_bit_used(waitCycle)),
            TOTAL_BIT_SIZE(_cnt_bit_sz + 1),
            _idle_cnt    (&makeOprVal("IdleCnt",TOTAL_BIT_SIZE, 0)),
            _start_cnt  (&makeOprVal("startCnt",TOTAL_BIT_SIZE, START_VAL)),
            _end_cnt    (&makeOprVal("endCnt",_cnt_bit_sz, waitCycle))
     {

        /** TO FIX*/
        com_init();
        assert(_cnt_bit_sz > 0);
     }

    CycleWaitStateReg::CycleWaitStateReg(Operable* endCnt):
            CtrlFlowRegBase(
        endCnt->getOperableSlice().getSize() + 1,
        false,
        TYPE_CYCLE_WAIT_STATE_REG,
        false
    ),
            _cnt_bit_sz     (endCnt->getOperableSlice().getSize()),
            TOTAL_BIT_SIZE(_cnt_bit_sz + 1),
            _idle_cnt    (&makeOprVal("IdleCnt", TOTAL_BIT_SIZE, 0)),
            _start_cnt  (&makeOprVal("startCnt", TOTAL_BIT_SIZE, START_VAL)),
            _end_cnt    (endCnt)
    {
        com_init();
        assert(_cnt_bit_sz > 0);
        /** generate update event for reset register*/
    }


    void CycleWaitStateReg::make_inc_state_event(Operable* holdSignal, CLOCK_MODE cm) {

        Operable* incCond = &((*this)(1, TOTAL_BIT_SIZE) != (*_end_cnt));
        if (holdSignal != nullptr){
            incCond = &((*holdSignal) & (*incCond));
        }

        auto* event = create_ue(
            incCond,
            &((*this)(0)),
            &((*this)(1, TOTAL_BIT_SIZE) + 1),
            Slice({1, TOTAL_BIT_SIZE}),
            DEFAULT_UE_PRI_INTERNAL_MAX-1,
            cm
        );
        addUpdateMeta(event);
    }

    void CycleWaitStateReg::com_init() {
        _ctrl->on_sp_reg_init(this, SP_CYCLE_WAIT_REG);
    }

    UpdateEventBase* CycleWaitStateReg::add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        assert(dependState != nullptr);
        auto* event = create_ue(activateCond,
                               dependState,
                               _start_cnt,
                               Slice({0, TOTAL_BIT_SIZE}),
                               DEFAULT_UE_PRI_INTERNAL_MAX,
                               cm
        );
        addUpdateMeta(event);
        return event;
    }

    void CycleWaitStateReg::make_un_set_state_event(CLOCK_MODE cm) {
        /**reset event*/
        auto* resetEvent = create_ue(
            &((*this)(1, TOTAL_BIT_SIZE) == (*_end_cnt)),
            &(*this)(0),
            _idle_cnt,
            Slice({0, TOTAL_BIT_SIZE}),
            DEFAULT_UE_PRI_INTERNAL_MIN,
            cm);
        addUpdateMeta(resetEvent);

    }

    void CycleWaitStateReg::make_user_rst_event(Operable* rst, CLOCK_MODE cm){
        /**reset event*/
        assert(rst != nullptr);
        auto* resetEvent = create_ue(
           nullptr,
           rst,
           _idle_cnt,
           Slice({0, TOTAL_BIT_SIZE}),
           DEFAULT_UE_PRI_INTERNAL_MIN,
            cm);
        addUpdateMeta(resetEvent);

    }

    Operable* CycleWaitStateReg::generate_end_expr() {
        return &((*this)(0) & ((*this)(1, TOTAL_BIT_SIZE) == (*_end_cnt)));
    }


}