//
// Created by tanawin on 11/2/2567.
//

#include"flowIdentifiable.h"
#include"flowBlock_Base.h"

namespace kathryn{


    FlowIdentifiable::FlowIdentifiable(std::string name) {
        _globalId = LAST_FLOW_IDENT_ID;
        _globalName = name + std::to_string(_globalId);
    }

    void FlowIdentifiable::setParent(FlowBlockBase *parentFlowBlock) {

        isParrentAssignYet = true;
        _parent = parentFlowBlock;
        if (parentFlowBlock != nullptr){
            _inheritName = _parent->getInheritName();
        }
        _inheritName.push_back(_globalName);
    }




}