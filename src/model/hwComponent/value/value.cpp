//
// Created by tanawin on 30/11/2566.
//

#include "value.h"
#include "model/controller/controller.h"


namespace kathryn{

Val::Val(int size, std::string v): Operable(),
                                   Identifiable(TYPE_VAL),
                                   HwCompControllerItf(),
                                   _size(size)
                                   {
                                       com_init();
}

void Val::com_init() {
    ctrl->on_value_init(this);
}


}