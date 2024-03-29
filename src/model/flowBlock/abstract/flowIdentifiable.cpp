//
// Created by tanawin on 11/2/2567.
//

#include"flowIdentifiable.h"
#include"flowBlock_Base.h"
#include"model/hwComponent/module/module.h"

namespace kathryn{

    FlowIdentifiable::FlowIdentifiable(const std::string& localName):
    IdentBase()
    {
        _globalName = localName + std::to_string(_globalId);
    }

    void FlowIdentifiable::setParent(FlowBlockBase *parentFlowBlock) {
        assert(parentFlowBlock != nullptr);
        _parentFb = parentFlowBlock;
    }

    void FlowIdentifiable::setParent(Module* parentModule){
        assert(parentModule != nullptr);
        _parentMod = parentModule;
    }



    void FlowIdentifiable::buildInheritName() {
        /** please remind that we need set Parent before use this function*/
        if (_parentFb != nullptr){
            _inheritName = _parentFb->getInheritName();

        }else if (_parentMod != nullptr){
            _inheritName = _parentMod->getInheritName();
        }else{
            assert(false);
        }
        _inheritName.push_back(_globalName);
        setIdentIsFinalize();

    }

    FlowBlockBase* FlowIdentifiable::getFlowBlockParrent() {
        return _parentFb;
    }

    Module* FlowIdentifiable::getModuleParent() {
        assert(_parentMod != nullptr);
        return _parentMod;
    }


}