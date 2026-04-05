//
// Created by tanawin on 3/10/2024.
//

#include "interface.h"
#include "model/controller/controller.h"


namespace kathryn{

    void ModelInterface::com_init(){
        _ctrl->on_itf_init(this);
    }


}