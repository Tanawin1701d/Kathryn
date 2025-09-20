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
    _condOpr(condOpr)
    {
        /** init comunication to system*/
        assert(condOpr != nullptr);
        com_init();
        /** generate update event for exit event*/

    }

    void CondWaitStateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_COND_WAIT_REG);
    }

    UpdateEvent* CondWaitStateReg::addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       &_upState,
                                       Slice({0, 1}),
                                       DEFAULT_UE_PRI_INTERNAL_MAX,
                                        DEFAULT_UE_SUB_PRIORITY_USER,
                                        cm
        });
        addUpdateMeta(event);
        return event;
    }

    void CondWaitStateReg::makeUnSetStateEvent(CLOCK_MODE cm) {
        auto* resetEvent = new UpdateEvent({    _condOpr,
                                                &((*this) == _upState),
                                                &_downState,
                                                Slice({0,1}),
                                                DEFAULT_UE_PRI_INTERNAL_MIN,
                                                DEFAULT_UE_SUB_PRIORITY_USER,
                                                cm
                                           });
        addUpdateMeta(resetEvent);
    }

    void CondWaitStateReg::makeUserRstEvent(Operable* rst, CLOCK_MODE cm){
        assert(rst != nullptr);
        auto* resetEvent = new UpdateEvent({    nullptr,
                                                rst,
                                                &_downState,
                                                Slice({0,1}),
                                                DEFAULT_UE_PRI_INTERNAL_MIN,
                                                DEFAULT_UE_SUB_PRIORITY_USER,
                                                cm
                                           });
        addUpdateMeta(resetEvent);
    }

    Operable* CondWaitStateReg::generateEndExpr() {
        return &((*_condOpr) & ((*this) == _upState));
    }

    /**
     *
     * cycle count wait state register
     *
     * */

    /** constructor for specific cycle*/
    CycleWaitStateReg::CycleWaitStateReg(int waitCycle):
            CtrlFlowRegBase( calBitUsed(waitCycle) + 1,
         false,
         TYPE_CYCLE_WAIT_STATE_REG,
         false
     ),
            _waitCycle(waitCycle),
            _cntBitSz     (calBitUsed(waitCycle)),
            _totalBitSize(_cntBitSz + 1),
            IdleCnt    (&makeOprVal("IdleCnt",_totalBitSize, 0)),
            _startCnt  (&makeOprVal("startCnt",_totalBitSize, startVal)),
            _endCnt    (&makeOprVal("endCnt",_cntBitSz, waitCycle))
     {

        /** TO FIX*/
        com_init();
        assert(_cntBitSz > 0);
     }

    CycleWaitStateReg::CycleWaitStateReg(Operable* endCnt):
            CtrlFlowRegBase(
        endCnt->getOperableSlice().getSize() + 1,
        false,
        TYPE_CYCLE_WAIT_STATE_REG,
        false
    ),
            _cntBitSz     (endCnt->getOperableSlice().getSize()),
            _totalBitSize(_cntBitSz + 1),
            IdleCnt    (&makeOprVal("IdleCnt", _totalBitSize, 0)),
            _startCnt  (&makeOprVal("startCnt", _totalBitSize, startVal)),
            _endCnt    (endCnt)
    {
        com_init();
        assert(_cntBitSz > 0);
        /** generate update event for reset register*/
    }


    void CycleWaitStateReg::makeIncStateEvent(Operable* holdSignal) {

        Operable* incCond = &((*this)(1, _totalBitSize) != (*_endCnt));
        if (holdSignal != nullptr){
            incCond = &((*holdSignal) & (*incCond));
        }

        auto* event = new UpdateEvent({
            incCond,
            &((*this)(0)),
            &((*this)(1, _totalBitSize) + 1),
            Slice({1, _totalBitSize}),
            DEFAULT_UE_PRI_INTERNAL_MAX-1
        });
        addUpdateMeta(event);
    }

    void CycleWaitStateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_CYCLE_WAIT_REG);
    }

    UpdateEvent* CycleWaitStateReg::addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       _startCnt,
                                       Slice({0, _totalBitSize}),
                                       DEFAULT_UE_PRI_INTERNAL_MAX,
                                        DEFAULT_UE_SUB_PRIORITY_USER,
                                        cm
        });
        addUpdateMeta(event);
        return event;
    }

    void CycleWaitStateReg::makeUnSetStateEvent(CLOCK_MODE cm) {
        /**reset event*/
        auto* resetEvent = new UpdateEvent({
            &((*this)(1, _totalBitSize) == (*_endCnt)),
            &(*this)(0),
            IdleCnt,
            Slice({0, _totalBitSize}),
            DEFAULT_UE_PRI_INTERNAL_MIN,
            DEFAULT_UE_SUB_PRIORITY_USER,
            cm
                                           });
        addUpdateMeta(resetEvent);

    }

    void CycleWaitStateReg::makeUserRstEvent(Operable* rst, CLOCK_MODE cm){
        /**reset event*/
        assert(rst != nullptr);
        auto* resetEvent = new UpdateEvent({
           nullptr,
           rst,
           IdleCnt,
           Slice({0, _totalBitSize}),
           DEFAULT_UE_PRI_INTERNAL_MIN,
            DEFAULT_UE_SUB_PRIORITY_USER,
            cm
                                           });
        addUpdateMeta(resetEvent);

    }

    Operable* CycleWaitStateReg::generateEndExpr() {
        return &((*this)(0) & ((*this)(1, _totalBitSize) == (*_endCnt)));
    }


}