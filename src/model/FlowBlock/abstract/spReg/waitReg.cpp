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
        com_init();
        /** generate update event for exit event*/

    }

    void CondWaitStateReg::com_init() {
        ctrl->on_cond_wait_reg_init(this);
    }

    UpdateEvent* CondWaitStateReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       &_upState,
                                       Slice({0, 1}),
                                       9});
        addUpdateMeta(event);
        return event;
    }

    void CondWaitStateReg::makeResetEvent() {
        auto* resetEvent = new UpdateEvent({    _condOpr,
                                                (&((*this) == _upState)),
                                                &_downState,
                                                Slice({0,1}),
                                                9
                                           });
        addUpdateMeta(resetEvent);
    }

    Operable* CondWaitStateReg::generateEndExpr() {
        return (Operable*) (&( (*_condOpr) & ((*this) == _upState) ) );
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
    _IdleCnt   (new Val(_bitSz,"d0")),
    _startCnt  (new Val(_bitSz,"d1")),
    _endCnt    (new Val(_bitSz,"d" +  std::to_string(waitCycle)))
     {
        com_init();
        assert(_bitSz > 0);
     }

    CycleWaitStateReg::CycleWaitStateReg(Operable* endCnt):
    CtrlFlowRegBase(
        endCnt->getOperableSlice().getSize(),
        false,
        TYPE_CYCLE_WAIT_STATE_REG,
        false
    ),
    _bitSz(endCnt->getOperableSlice().getSize()),
    _IdleCnt   (new Val(_bitSz,"d0")),
    _startCnt  (new Val(_bitSz,"d1")),
    _endCnt    (endCnt)
    {
        com_init();
        assert(_bitSz > 0);
        /** generate update event for reset register*/
    }

    void CycleWaitStateReg::com_init() {
        ctrl->on_cycle_wait_reg_init(this);
    }

    UpdateEvent* CycleWaitStateReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       _startCnt,
                                       Slice({0, _bitSz}),
                                       9});
        addUpdateMeta(event);
        return event;
    }

    void CycleWaitStateReg::makeResetEvent() {
        /**reset event*/
        auto* resetEvent = new UpdateEvent({
                                                   nullptr,
                                                   (&((*this) == (*_endCnt))),
                                                   _IdleCnt,
                                                   Slice({0, _bitSz}),
                                                   9
                                           });
        addUpdateMeta(resetEvent);

    }

    Operable* CycleWaitStateReg::generateEndExpr() {
        return &((*this) == (*_endCnt));
    }



}