//
// Created by tanawin on 2/12/2566.
//

#include "flowBlock_Base.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type):
    _type(type),
    ctrl(getControllerPtr()) {}

    FlowBlockBase::~FlowBlockBase(){
        for (auto sub_fb: subBlocks){
            delete sub_fb;

        }
    }


}