//
// Created by tanawin on 11/2/2567.
//

#include"flowIdentifiable.h"
#include"flowBlock_Base.h"
#include"model/hwComponent/module/module.h"

namespace kathryn{

    ull LAST_FLOW_IDENT_ID = 0;

    FlowIdentifiable::FlowIdentifiable(std::string name) {
        _globalId = LAST_FLOW_IDENT_ID;
        _globalName = name + std::to_string(_globalId);
    }

    void FlowIdentifiable::setParent(FlowBlockBase *parentFlowBlock) {
        assert(parentFlowBlock != nullptr);
        isParentFbAssignYet = true;
        _parentFb = parentFlowBlock;
    }

    void FlowIdentifiable::setParent(Module* parentModule){
        assert(parentModule != nullptr);
        isParentMdAssignYet = true;
        _parentMod = parentModule;

    }

    void FlowIdentifiable::assignInheritName() {
        /** please remind that we need set Parent before use this function*/
        if (_parentFb != nullptr){
            _inheritName = _parentFb->getInheritName();

        }else if (_parentMod != nullptr){
            _inheritName = _parentMod->getInheritName();
        }else{
            assert(false);
        }
        _inheritName.push_back(_globalName);
        isInheritNameAssignYet = true;
    }




}