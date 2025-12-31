//
// Created by tanawin on 6/1/2567.
//

#include <utility>

#include "model/controller/controller.h"
#include "model/hwComponent/abstract/identifiable.h"


namespace kathryn{


    void unlockAlloc(){
        getControllerPtr()->unlockAllocation();
    }


}