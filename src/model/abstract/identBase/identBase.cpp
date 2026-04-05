//
// Created by tanawin on 29/3/2567.
//

#include "identBase.h"

namespace kathryn{


    ull GLOBAL_MODEL_ID = 0;

    ull get_last_ident_id(){
        return GLOBAL_MODEL_ID;
    }


    IdentBase::IdentBase() {

        _global_id = GLOBAL_MODEL_ID++;

        if (_global_id == 92){
            int dsdgdfgfd= 5;
        }
    }


    bool IdentBase::is_ident_finalized() const {
        return _is_finalized;
    }

    void IdentBase::finalize_ident() {
        IdentBase::_is_finalized = true;
    }

    const std::string &IdentBase::get_global_name() const {
        return _global_name;
    }

    void IdentBase::set_global_name(const std::string &global_name) {
        _global_name = global_name;
    }

    const std::vector<std::string> &IdentBase::get_inherit_name() const {
        return _inherit_name;
    }

    void IdentBase::set_inherit_name(const std::vector<std::string> &inherit_name) {
        _inherit_name = inherit_name;
    }

    std::string IdentBase::concat_inheritName() {
        assert(_is_finalized);
        std::string pre_ret;
        for (const auto& str: _inherit_name){
            pre_ret += str + "_";
        }
        assert(pre_ret.size() > 1);
        return pre_ret.substr(0, pre_ret.size()-1);
    }

    IdentBase& IdentBase::operator = (const IdentBase& rhs){
        if (this == &rhs){
            return *this;
        }
        _is_finalized  = rhs._is_finalized;
        _global_id     = GLOBAL_MODEL_ID++;
        _global_name   = rhs._global_name + "_CP";
        _inherit_name  = rhs._inherit_name;
        _inherit_name.emplace_back("_CP");
        return *this;
    }

}