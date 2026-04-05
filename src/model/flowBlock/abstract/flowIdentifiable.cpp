//
// Created by tanawin on 11/2/2567.
//

#include "flowIdentifiable.h"
#include "flowBlockBase.h"
#include "model/hwComponent/module/module.h"

namespace kathryn{

    FlowIdentifiable::FlowIdentifiable(const std::string& localName):
    IdentBase()
    {
        _globalName = localName + std::toString(_globalId);
    }

    void FlowIdentifiable::setParent(FlowBlockBase *parentFlowBlock) {
        assert(parentFlowBlock != nullptr);
        _parentFb = parentFlowBlock;
    }

    void FlowIdentifiable::setParent(Module* parentModule){
        assert(parentModule != nullptr);
        _parentMod = parentModule;
    }

    void FlowIdentifiable::setZepTrackName(const std::string& zepTrackName){
        _isZepTrackName = true;
        _zepTrackName   = zepTrackName;
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
        _inheritName.pushBack(_globalName);
        setIdentIsFinalize();

    }

    FlowBlockBase* FlowIdentifiable::getFlowBlockParrentPtr() {
        return _parentFb;
    }

    Module* FlowIdentifiable::getModuleParentPtr() {
        assert(_parentMod != nullptr);
        return _parentMod;
    }

    bool FlowIdentifiable::isZepTrackNameSet(){
        return _isZepTrackName;
    }


    std::string FlowIdentifiable::getZepTrackName(){
        assert(_isZepTrackName);
        return _zepTrackName;
    }





}