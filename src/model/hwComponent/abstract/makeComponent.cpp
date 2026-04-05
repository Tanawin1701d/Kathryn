//
// Created by tanawin on 6/1/2567.
//

#include <utility>

#include "model/controller/controller.h"
#include "model/hwComponent/abstract/identifiable.h"


namespace kathryn{


    void unlockAlloc(){
        get_controller_ptr()->unlock_allocation();
    }


}