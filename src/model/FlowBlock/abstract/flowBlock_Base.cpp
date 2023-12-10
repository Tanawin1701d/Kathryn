//
// Created by tanawin on 2/12/2566.
//

#include "flowBlock_Base.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"

namespace kathryn{


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type):
    _type(type),
    ctrl(getControllerPtr()),
    lazyDeletedRequired(false)
    {}

    FlowBlockBase::~FlowBlockBase(){
        for (auto sub_fb: subBlocks){
            delete sub_fb;
        }
        ///// we don't delete basicNode because it is used by other fb
    }

    FlowBlockBase* FlowBlockBase::genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType) {
        /** determine next flow block*/
        FLOW_BLOCK_TYPE nextFbType = ctrl->get_top_pattern_flow_block_type();
        if (nextFbType == DUMMY_BLOCK){
            nextFbType = defaultType;
        }
        /** create subblock*/
        if (nextFbType == PARALLEL){
            return new FlowBlockPar();
        }else if (nextFbType == SEQUENTIAL){
            return new FlowBlockSeq();
        }else{
            assert(true); /** can't determine flow type*/
        }
        return nullptr;
    }


}