//
// Created by tanawin on 1/12/2566.
//

#include "controllerItf.h"

#include "model/controller/controller.h"


namespace kathryn{
    HwCompControllerItf::HwCompControllerItf(bool required_alloc_check): _ctrl(get_controller_ptr()) {

        if (required_alloc_check) {
            assert(!_ctrl->is_allocation_lock());
            _ctrl->lock_allocation();
        }
    }
}
