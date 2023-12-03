//
// Created by tanawin on 1/12/2566.
//

#include "controllerItf.h"

#include "model/controller/controller.h"


namespace kathryn{
    HwCompControllerItf::HwCompControllerItf(): ctrl(getControllerPtr()) {}
}
