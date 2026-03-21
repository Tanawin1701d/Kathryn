//
// Created by tanawin on 11/2/2567.
//

#include "flow_identifiable.h"
#include "flow_block__base.h"
#include "model/hw_component/module/module.h"

namespace kathryn{

    FlowIdentifiable::FlowIdentifiable(const std::string& local_name):
    IdentBase()
    {
        _globalName = local_name + std::to_string(_globalId);
    }

    void FlowIdentifiable::set_parent(FlowBlockBase *parent_flow_block) {
        assert(parent_flow_block != nullptr);
        _parentFb = parent_flow_block;
    }

    void FlowIdentifiable::set_parent(Module* parent_module){
        assert(parent_module != nullptr);
        _parentMod = parent_module;
    }

    void FlowIdentifiable::set_zep_track_name(const std::string& zep_track_name){
        _isZepTrackName = true;
        _zepTrackName   = zep_track_name;
    }




    void FlowIdentifiable::build_inherit_name() {
        /** please remind that we need set Parent before use this function*/
        if (_parentFb != nullptr){
            _inheritName = _parentFb->get_inherit_name();

        }else if (_parentMod != nullptr){
            _inheritName = _parentMod->get_inherit_name();
        }else{
            assert(false);
        }
        _inheritName.push_back(_globalName);
        set_ident_is_finalize();

    }

    FlowBlockBase* FlowIdentifiable::get_flow_block_parrent_ptr() {
        return _parentFb;
    }

    Module* FlowIdentifiable::get_module_parent_ptr() {
        assert(_parentMod != nullptr);
        return _parentMod;
    }

    bool FlowIdentifiable::is_zep_track_name_set(){
        return _isZepTrackName;
    }


    std::string FlowIdentifiable::get_zep_track_name(){
        assert(_isZepTrackName);
        return _zepTrackName;
    }





}