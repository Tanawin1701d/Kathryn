//
// Created by tanawin on 5/12/2566.
//

#include "stateReg.h"

#include "model/controller/controller.h"
#include "updateEvent.h"

namespace kathryn {

    void StateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_STATE_REG);
    }

    StateReg::StateReg(): CtrlFlowRegBase(        1,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
              upFullState  (_make<Val>("upFullState"  ,1, "1b1")),
              downFullState(_make<Val>("downFullState",1, "1b0"))
    {
        com_init();
    };

    UpdateEvent* StateReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       &upFullState,
                                       Slice({0, 1}),
                                       9});
        addUpdateMeta(event);
        return event;
    }

    void StateReg::makeUnSetStateEvent() {
        auto* event = new UpdateEvent({
            nullptr,
            &((*this) == upFullState),
            &downFullState,
            Slice({0, getSlice().getSize()}),
            8
        });
        addUpdateMeta(event);
    }

    Operable* StateReg::generateEndExpr(){
        return (&((*this) == upFullState));
    }

}