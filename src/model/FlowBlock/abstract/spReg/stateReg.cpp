//
// Created by tanawin on 5/12/2566.
//

#include "stateReg.h"

#include "model/controller/controller.h"


namespace kathryn {

    void StateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_STATE_REG);
    }

    StateReg::StateReg(): CtrlFlowRegBase(        1,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
              upFullState  (_make<Val>("stateRegUpFull"  ,1, 1)),
              downFullState(_make<Val>("stateRegDownFull",1, 0))
    {
        com_init();
    };

    UpdateEvent* StateReg::addDependState(Operable* dependState, Operable* activateCond){
        assert(dependState != nullptr);
        auto* event = new UpdateEvent({activateCond,
                                       dependState,
                                       &upFullState,
                                       Slice({0, 1}),
                                       DEFAULT_UE_PRI_INTERNAL_MAX});
        addUpdateMeta(event);
        return event;
    }

    void StateReg::makeUnSetStateEvent() {
        auto* event = new UpdateEvent({
            nullptr,
            &((*this) == upFullState),
            &downFullState,
            Slice({0, getSlice().getSize()}),
            DEFAULT_UE_PRI_INTERNAL_MIN
        });
        addUpdateMeta(event);
    }

    Operable* StateReg::generateEndExpr(){
        return (&((*this) == upFullState));
    }

}