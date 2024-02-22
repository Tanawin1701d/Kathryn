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

    UpdateEvent* CondWaitStateReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       &_upState,
                                       Slice({0, 1}),
                                       DEFAULT_UE_PRI_INTERNAL_MAX});
        addUpdateMeta(event);
        return event;
    }

    void CondWaitStateReg::makeUnSetStateEvent() {
        auto* resetEvent = new UpdateEvent({    _condOpr,
                                                &((*this) == _upState),
                                                &_downState,
                                                Slice({0,1}),
                                                DEFAULT_UE_PRI_INTERNAL_MIN
                                           });
        addUpdateMeta(resetEvent);
    }

    Operable* CondWaitStateReg::generateEndExpr() {
        return &((*_condOpr) & ((*this) == _upState));
    }

    bool CondWaitStateReg::isSimAtWaiting(){
        /** register sim*/
        assert(getRtlValItf()->isCurValSim());
        ValRep& curVal = getRtlValItf()->getCurVal();
        /** val sim*/
        assert(_downState.getRtlValItf()->isCurValSim());
        ValRep& restCurVal = _downState.getRtlValItf()->getCurVal();

        return (curVal > restCurVal).getLogicalValue();
    }

    /**
     *
     * cycle count wait state register
     *
     * */

    /** constructor for specific cycle*/
    CycleWaitStateReg::CycleWaitStateReg(int waitCycle):
    CtrlFlowRegBase( calBitUsed(waitCycle),
         false,
         TYPE_CYCLE_WAIT_STATE_REG,
         false
     ),
     _waitCycle(waitCycle),
    _bitSz     (calBitUsed(waitCycle)),
    IdleCnt    (&_make<Val>("IdleCnt" ,_bitSz,0        )),
    _startCnt  (&_make<Val>("startCnt",_bitSz,1        )),
    _endCnt    (&_make<Val>("endCnt"  ,_bitSz,waitCycle))
     {
        /** TO FIX*/
        com_init();
        makeIncStateEvent();
        assert(_bitSz > 0);
     }

    CycleWaitStateReg::CycleWaitStateReg(Operable* endCnt):
    CtrlFlowRegBase(
        endCnt->getOperableSlice().getSize(),
        false,
        TYPE_CYCLE_WAIT_STATE_REG,
        false
    ),
    _bitSz     (endCnt->getOperableSlice().getSize()),
    IdleCnt    (&_make<Val>("IdleCnt" ,_bitSz,0)),
    _startCnt  (&_make<Val>("startCnt",_bitSz,1)),
    _endCnt    (endCnt)
    {
        com_init();
        makeIncStateEvent();
        assert(_bitSz > 0);
        /** generate update event for reset register*/
    }


    void CycleWaitStateReg::makeIncStateEvent() {
        auto* event = new UpdateEvent({
            nullptr,
            &(((*this) < (*_endCnt)) & ((*this) >= (*_startCnt))),
            &((*this) + (*_startCnt)),
            Slice({0, _bitSz}),
            DEFAULT_UE_PRI_INTERNAL_MAX-1
        });
        addUpdateMeta(event);
    }

    void CycleWaitStateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_CYCLE_WAIT_REG);
    }

    UpdateEvent* CycleWaitStateReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       _startCnt,
                                       Slice({0, _bitSz}),
                                       DEFAULT_UE_PRI_INTERNAL_MAX});
        addUpdateMeta(event);
        return event;
    }

    void CycleWaitStateReg::makeUnSetStateEvent() {
        /**reset event*/
        auto* resetEvent = new UpdateEvent({
                                                   nullptr,
                                                   (&((*this) == (*_endCnt))),
                                                   IdleCnt,
                                                   Slice({0, _bitSz}),
                                                   DEFAULT_UE_PRI_INTERNAL_MIN
                                           });
        addUpdateMeta(resetEvent);

    }

    Operable* CycleWaitStateReg::generateEndExpr() {
        return &((*this) == (*_endCnt));
    }

    bool CycleWaitStateReg::isSimAtWaiting(){
        /** register sim*/
        assert(getRtlValItf()->isCurValSim());
        ValRep& curVal = getRtlValItf()->getCurVal();
        /** val sim*/
        assert(IdleCnt->getRtlValItf()->isCurValSim());
        ValRep& restCurVal = IdleCnt->getRtlValItf()->getCurVal();

        return (curVal > restCurVal).getLogicalValue();
    }

}