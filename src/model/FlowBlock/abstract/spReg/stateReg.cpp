//
// Created by tanawin on 5/12/2566.
//

#include "stateReg.h"

#include "model/controller/controller.h"

namespace kathryn {

    void StateReg::com_init() {
        ctrl->on_state_reg_init(this);
    }

    StateReg::StateReg(int size): CtrlFlowRegBase(size,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
                                  upState(1, "b1"),
                                  upFullState(size, genConseBinaryValue(true, size)),
                                  downFullState(size, genConseBinaryValue(false, size))
    {
        com_init();
    };

    UpdateEvent* StateReg::addDependStateUpdateEvent(Operable* dependStateCon, int bit){
        auto* event = new UpdateEvent({nullptr,
                                       dependStateCon,
                                       &upState,
                                       Slice({bit, bit + 1}),
                                       9});
        addUpdateMeta(event);
        return event;
    }

    void StateReg::makeResetEvent() {
        auto* event = new UpdateEvent({
            nullptr,
            &((*this) == upFullState),
            &downFullState,
            Slice({0, getSlice().getSize()}),
            9
        });
        addUpdateMeta(event);
    }

    Operable* StateReg::generateEndExpr(){
        return (Operable*)(&((*this) == upFullState));
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