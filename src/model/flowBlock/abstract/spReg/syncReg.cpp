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
                                upState      (makeOprVal("upState"      , 1  , 1)),
                                upFullState  (makeOprVal("upFullState"  , size,genBiConValRep(true , size))),
                                downFullState(makeOprVal("downFullState", size,genBiConValRep(false, size))),
                                testWire     (makeOprWire("testSyncWire", size)),
                                endExpr(&(((*this) | testWire) == upFullState)),
                                endExprInv(&(~(*endExpr))),
                                nextFillActivateId(0)
    {
        com_init();
        assert(size > 0);
    };

    UpdateEvent* SyncReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(activateCond == nullptr);
        assert(dependState != nullptr);
        /** if endExpr rise, it is neccessary to tel register to rise*/
        auto* event = new UpdateEvent({endExprInv,
                                       dependState,
                                       &upState,
                                       Slice({nextFillActivateId, nextFillActivateId + 1}),
                                       DEFAULT_UE_PRI_INTERNAL_MAX
        });
        addUpdateMeta(event);
        ////// assign observe wire
        auto* testEvent = new UpdateEvent({
            nullptr,
            dependState,
            &upState,
            Slice({nextFillActivateId, nextFillActivateId + 1}),
            DEFAULT_UE_PRI_INTERNAL_MAX
        });
        testWire.addUpdateMeta(testEvent);

        nextFillActivateId++;
        assert(nextFillActivateId <= getSlice().getSize());

        return event;
    }

    void SyncReg::makeUnSetStateEvent() {

        ////// unset also testExpr
        auto* event = new UpdateEvent({
            nullptr,
            &(((*this) | testWire) == upFullState),
            &downFullState,
            Slice({0, getSlice().getSize()}),
            DEFAULT_UE_PRI_INTERNAL_MIN
        });
        addUpdateMeta(event);
    }

    void SyncReg::makeUserRstEvent(Operable* userRst) {
        assert(userRst != nullptr);
        auto* event = new UpdateEvent({
          nullptr,
          userRst,
          &downFullState,
          Slice({0, getSlice().getSize()}),
          DEFAULT_UE_PRI_INTERNAL_MIN////// we priority set event first rst must be lower
                                      });
        addUpdateMeta(event);
    }

    Operable* SyncReg::generateEndExpr(){
        assert(nextFillActivateId == getOperableSlice().getSize());
        return endExpr;
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