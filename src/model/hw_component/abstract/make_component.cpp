//
// Created by tanawin on 6/1/2567.
//

#include "utility"

#include "model/controller/controller.h"
#include "model/hw_component/abstract/identifiable.h"


namespace kathryn{


    void unlock_alloc(){
        get_controller_ptr()->unlock_allocation();
    }


}