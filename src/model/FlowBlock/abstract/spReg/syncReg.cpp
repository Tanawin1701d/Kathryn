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
              upState      (_make<Val>("upState"      , 1  , "b1")),
              upFullState  (_make<Val>("upFullState"  ,size, genConseBinaryValue(true, size))),
              downFullState(_make<Val>("downFullState",size, genConseBinaryValue(false, size))),
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
                                       9});
        nextFillActivateId++;
        assert(nextFillActivateId <= getSlice().getSize());
        addUpdateMeta(event);
        return event;
    }

    void SyncReg::makeUnSetStateEvent() {
        auto* event = new UpdateEvent({
            nullptr,
            &((*this) == upFullState),
            &downFullState,
            Slice({0, getSlice().getSize()}),
            8
        });
        addUpdateMeta(event);
    }

    Operable* SyncReg::generateEndExpr(){
        return (&((*this) == upFullState));
    }

    bool SyncReg::isSimAtFullSyn() {
        /** register sim */
        RtlSimEngine* regEnginePtr = getSimEngine();
        ValRep& curVal = regEnginePtr->getCurVal();
        /** val sim*/
        RtlSimEngine* valEnginePtr = upFullState.getSimEngine();
        ValRep& fullStateRep       = valEnginePtr->getCurVal();
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