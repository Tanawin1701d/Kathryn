//
// Created by tanawin on 5/12/2566.
//

#include "syncReg.h"
#include "model/controller/controller.h"

namespace kathryn {

    void SyncReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_SYNC_REG);
    }

    SyncReg::SyncReg(int size): CtrlFlowRegBase(size,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
              upState      (_make<Val>("upState"      , 1  , 1)),
              upFullState  (_make<Val>("upFullState"  ,genBiConValRep(true , size))),
              downFullState(_make<Val>("downFullState",genBiConValRep(false, size))),
              testExpr(new expression(size)),
              nextFillActivateId(0)
    {
        com_init();
    };

    UpdateEvent* SyncReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       &upState,
                                       Slice({nextFillActivateId, nextFillActivateId + 1}),
                                       DEFAULT_UE_PRI_INTERNAL_MAX});

        ////// assign observe wire
        if (activateCond == nullptr) {
            (*testExpr)(nextFillActivateId) = *dependState;
        }else{
            (*testExpr)(nextFillActivateId) = (*dependState) & (*activateCond);
        }
        addUpdateMeta(event);

        nextFillActivateId++;
        assert(nextFillActivateId <= getSlice().getSize());

        return event;
    }

    void SyncReg::makeUnSetStateEvent() {

        ////// unset also testExpr
        auto* event = new UpdateEvent({
            nullptr,
            &(((*this) | (*testExpr)) == upFullState),
            &downFullState,
            Slice({0, getSlice().getSize()}),
            DEFAULT_UE_PRI_INTERNAL_MIN
        });
        addUpdateMeta(event);
    }

    Operable* SyncReg::generateEndExpr(){
        return &(((*this) | (*testExpr)) == upFullState);
    }

    bool SyncReg::isSimAtFullSyn() {
        /** register sim */
        assert(getRtlValItf()->isCurValSim());
        ValRep& curVal = getRtlValItf()->getCurVal();
        /** val sim*/
        assert(upFullState.getRtlValItf()->isCurValSim());
        ValRep& fullStateRep       = upFullState.getRtlValItf()->getCurVal();
        /**due to it return valrep we must check it is equal to 1*/
        return (curVal == fullStateRep).getLogicalValue();
    }


    std::string genConseBinaryValue(bool bitVal, int size){

        std::string retString = "b";
        std::string fillVal = bitVal ? "1" : "0";
        for (int i = 0; i < size; i++ ){
            retString += fillVal;
        }
        return retString;

    }

}