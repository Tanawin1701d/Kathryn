//
// Created by tanawin on 5/12/2566.
//

#include "stateReg.h"

#include "model/controller/controller.h"


namespace kathryn {

    void StateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_STATE_REG);
    }

    StateReg::StateReg(bool rstReq): CtrlFlowRegBase(1,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
              _rstReq(rstReq)
    {
        com_init();
    };

    UpdateEventBase* StateReg::addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        assert(dependState != nullptr);

        // if (_globalId == 18){
        //     std::cout << "addDependState " << _globalId << std::endl;
        // }


        UpdateEventBase* conEvent = createUE(activateCond,
                                             dependState,
                                             &stateRegUpFull,
                                             Slice({0, 1}),
                                             DEFAULT_UE_PRI_INTERNAL_MAX,
                                             cm);
        addUpdateMeta(conEvent);
        return conEvent;

        // auto* event = new UpdateEvent({activateCond,
        //                                dependState,
        //                                &stateRegUpFull,
        //                                Slice({0, 1}),
        //                                DEFAULT_UE_PRI_INTERNAL_MAX,
        //                                 DEFAULT_UE_SUB_PRIORITY_USER,
        //                                 cm
        //                                });

    }

    void StateReg::makeUnSetStateEvent(CLOCK_MODE cm) {

         auto* event =   createUE(nullptr,
                         this,
                         &stateRegDownFull,
                         Slice({0, getSlice().getSize()}),
                         DEFAULT_UE_PRI_INTERNAL_MIN,
                         cm);
        //////// add update Meta
        addUpdateMeta(event);

        // auto* event = new UpdateEvent({
        //     nullptr,
        //     this,
        //     &stateRegDownFull,
        //     Slice({0, getSlice().getSize()}),
        //     DEFAULT_UE_PRI_INTERNAL_MIN,
        //     DEFAULT_UE_SUB_PRIORITY_USER,
        //     cm
        // });
        // addUpdateMeta(event);
    }

    Operable* StateReg::generateEndExpr(){
        return this;
    }

}