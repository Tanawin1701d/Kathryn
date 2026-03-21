//
// Created by tanawin on 5/12/2566.
//

#include "state_reg.h"

#include "model/controller/controller.h"


namespace kathryn {

    void StateReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_STATE_REG);
    }

    StateReg::StateReg(bool rst_req): CtrlFlowRegBase(1,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
              _rstReq(rst_req)
    {
        com_init();
    };

    UpdateEventBase* StateReg::add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm){
        assert(depend_state != nullptr);

        // if (_globalId == 18){
        //     std::cout << "add_depend_state " << _globalId << std::endl;
        // }


        UpdateEventBase* con_event = create_ue(activate_cond,
                                             depend_state,
                                             &state_reg_up_full,
                                             Slice({0, 1}),
                                             DEFAULT_UE_PRI_INTERNAL_MAX,
                                             cm);
        add_update_meta(con_event);
        return con_event;

        // auto* event = new UpdateEvent({activate_cond,
        //                                depend_state,
        //                                &state_reg_up_full,
        //                                Slice({0, 1}),
        //                                DEFAULT_UE_PRI_INTERNAL_MAX,
        //                                 DEFAULT_UE_SUB_PRIORITY_USER,
        //                                 cm
        //                                });

    }

    void StateReg::make_un_set_state_event(CLOCK_MODE cm) {

         auto* event =   create_ue(nullptr,
                         this,
                         &state_reg_down_full,
                         Slice({0, get_slice().get_size()}),
                         DEFAULT_UE_PRI_INTERNAL_MIN,
                         cm);
        //////// add update Meta
        add_update_meta(event);

        // auto* event = new UpdateEvent({
        //     nullptr,
        //     this,
        //     &state_reg_down_full,
        //     Slice({0, get_slice().get_size()}),
        //     DEFAULT_UE_PRI_INTERNAL_MIN,
        //     DEFAULT_UE_SUB_PRIORITY_USER,
        //     cm
        // });
        // add_update_meta(event);
    }

    Operable* StateReg::generate_end_expr(){
        return this;
    }

}