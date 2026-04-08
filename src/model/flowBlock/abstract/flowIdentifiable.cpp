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

    void FlowIdentifiable::set_parent(FlowBlockBase *parent_flow_block) {
        assert(parent_flow_block != nullptr);
        _parent_fb = parent_flow_block;
    }

    void FlowIdentifiable::set_parent(Module* parent_module){
        assert(parent_module != nullptr);
        _parent_mod = parent_module;
    }

    void FlowIdentifiable::set_zep_track_name(const std::string& zep_track_name){
        _is_zep_track_name_set = true;
        _user_zep_track_name   = zep_track_name;
    }




    void FlowIdentifiable::build_inherit_name() {
        /// please remind that we need set Parent before use this function
        if (_parent_fb != nullptr){
            _inherit_name = _parent_fb->get_inherit_name();

        }else if (_parent_mod != nullptr){
            _inherit_name = _parent_mod->get_inherit_name();
        }else{
            assert(false);
        }
        _inherit_name.push_back(_global_name);
        finalize_ident();

    }

    FlowBlockBase* FlowIdentifiable::get_flow_block_parent_ptr() {
        return _parent_fb;
    }

    Module* FlowIdentifiable::get_module_parent_ptr() {
        assert(_parent_mod != nullptr);
        return _parent_mod;
    }

    bool FlowIdentifiable::is_zep_track_name_set(){
        return _is_zep_track_name_set;
    }


    std::string FlowIdentifiable::get_zep_track_name(){
        assert(_is_zep_track_name_set);
        return _user_zep_track_name;
    }





}