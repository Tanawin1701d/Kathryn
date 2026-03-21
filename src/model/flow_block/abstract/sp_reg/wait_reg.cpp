//
// Created by tanawin on 3/1/2567.
//

#include "wait_reg.h"
#include "model/controller/controller.h"


namespace kathryn{


    /**
     *
     * conditional wait state register
     *
     * */


    CondWaitStateReg::CondWaitStateReg(Operable *cond_opr) :
    CtrlFlowRegBase(1,
                    false,
                    TYPE_COND_WAIT_STATE_REG,
                    false
    ),
    _condOpr(cond_opr)
    {
        /** init comunication to system*/
        assert(cond_opr != nullptr);
        com_init();
        /** generate update event for exit event*/

    }

    void CondWaitStateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_COND_WAIT_REG);
    }

    UpdateEventBase* CondWaitStateReg::add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm){
        assert(depend_state != nullptr);
        auto* event = create_ue(activate_cond,
                               depend_state,
                               &_upState,
                               Slice({0, 1}),
                               DEFAULT_UE_PRI_INTERNAL_MAX,
                               cm
        );
        add_update_meta(event);
        return event;
    }

    void CondWaitStateReg::make_un_set_state_event(CLOCK_MODE cm) {
        auto* reset_event = create_ue(_condOpr,
                                    &((*this) == _upState),
                                    &_downState,
                                    Slice({0,1}),
                                    DEFAULT_UE_PRI_INTERNAL_MIN,
                                    cm);
        add_update_meta(reset_event);
    }

    void CondWaitStateReg::make_user_rst_event(Operable* rst, CLOCK_MODE cm){
        assert(rst != nullptr);
        auto* reset_event = create_ue(nullptr,
                                    rst,
                                    &_downState,
                                    Slice({0,1}),
                                    DEFAULT_UE_PRI_INTERNAL_MIN,
                                    cm
                                    );
        add_update_meta(reset_event);
    }

    Operable* CondWaitStateReg::generate_end_expr() {
        return &((*_condOpr) & ((*this) == _upState));
    }

    /**
     *
     * cycle count wait state register
     *
     * */

    /** constructor for specific cycle*/
    CycleWaitStateReg::CycleWaitStateReg(int wait_cycle):
            CtrlFlowRegBase( cal_bit_used(wait_cycle) + 1,
         false,
         TYPE_CYCLE_WAIT_STATE_REG,
         false
     ),
            _waitCycle(wait_cycle),
            _cntBitSz     (cal_bit_used(wait_cycle)),
            _totalBitSize(_cntBitSz + 1),
            IdleCnt    (&make_opr_val("IdleCnt",_totalBitSize, 0)),
            _startCnt  (&make_opr_val("start_cnt",_totalBitSize, start_val)),
            _endCnt    (&make_opr_val("end_cnt",_cntBitSz, wait_cycle))
     {

        /** TO FIX*/
        com_init();
        assert(_cntBitSz > 0);
     }

    CycleWaitStateReg::CycleWaitStateReg(Operable* end_cnt):
            CtrlFlowRegBase(
        end_cnt->get_operable_slice().get_size() + 1,
        false,
        TYPE_CYCLE_WAIT_STATE_REG,
        false
    ),
            _cntBitSz     (end_cnt->get_operable_slice().get_size()),
            _totalBitSize(_cntBitSz + 1),
            IdleCnt    (&make_opr_val("IdleCnt", _totalBitSize, 0)),
            _startCnt  (&make_opr_val("start_cnt", _totalBitSize, start_val)),
            _endCnt    (end_cnt)
    {
        com_init();
        assert(_cntBitSz > 0);
        /** generate update event for reset register*/
    }


    void CycleWaitStateReg::make_inc_state_event(Operable* hold_signal, CLOCK_MODE cm) {

        Operable* inc_cond = &((*this)(1, _totalBitSize) != (*_endCnt));
        if (hold_signal != nullptr){
            inc_cond = &((*hold_signal) & (*inc_cond));
        }

        auto* event = create_ue(
            inc_cond,
            &((*this)(0)),
            &((*this)(1, _totalBitSize) + 1),
            Slice({1, _totalBitSize}),
            DEFAULT_UE_PRI_INTERNAL_MAX-1,
            cm
        );
        add_update_meta(event);
    }

    void CycleWaitStateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_CYCLE_WAIT_REG);
    }

    UpdateEventBase* CycleWaitStateReg::add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm){
        assert(depend_state != nullptr);
        auto* event = create_ue(activate_cond,
                               depend_state,
                               _startCnt,
                               Slice({0, _totalBitSize}),
                               DEFAULT_UE_PRI_INTERNAL_MAX,
                               cm
        );
        add_update_meta(event);
        return event;
    }

    void CycleWaitStateReg::make_un_set_state_event(CLOCK_MODE cm) {
        /**reset event*/
        auto* reset_event = create_ue(
            &((*this)(1, _totalBitSize) == (*_endCnt)),
            &(*this)(0),
            IdleCnt,
            Slice({0, _totalBitSize}),
            DEFAULT_UE_PRI_INTERNAL_MIN,
            cm);
        add_update_meta(reset_event);

    }

    void CycleWaitStateReg::make_user_rst_event(Operable* rst, CLOCK_MODE cm){
        /**reset event*/
        assert(rst != nullptr);
        auto* reset_event = create_ue(
           nullptr,
           rst,
           IdleCnt,
           Slice({0, _totalBitSize}),
           DEFAULT_UE_PRI_INTERNAL_MIN,
            cm);
        add_update_meta(reset_event);

    }

    Operable* CycleWaitStateReg::generate_end_expr() {
        return &((*this)(0) & ((*this)(1, _totalBitSize) == (*_endCnt)));
    }


}