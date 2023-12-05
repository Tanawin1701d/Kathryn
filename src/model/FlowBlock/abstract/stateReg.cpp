//
// Created by tanawin on 5/12/2566.
//

#include "stateReg.h"

#include "model/controller/controller.h"

namespace kathryn {

    void StateReg::com_init() {
        ctrl->on_state_reg_init(this);
    }



}