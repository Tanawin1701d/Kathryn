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
        _global_name = localName + std::to_string(_global_id);
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




    void FlowIdentifiable::build_inherit_name() {
        /** please remind that we need set Parent before use this function*/
        if (_parentFb != nullptr){
            _inherit_name = _parentFb->get_inherit_name();

        }else if (_parentMod != nullptr){
            _inherit_name = _parentMod->get_inherit_name();
        }else{
            assert(false);
        }
        _inherit_name.push_back(_global_name);
        finalize_ident();

    }

    FlowBlockBase* FlowIdentifiable::getFlowBlockParrent() {
        return _parentFb;
    }

    Module* FlowIdentifiable::getModuleParent() {
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