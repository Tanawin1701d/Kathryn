//
// Created by tanawin on 29/3/2567.
//

#include "ident_base.h"

namespace kathryn{


    ull GLOBAL_MODEL_ID = 0;

    ull get_last_ident_id(){
        return GLOBAL_MODEL_ID;
    }


    IdentBase::IdentBase() {

        _globalId = GLOBAL_MODEL_ID++;

        if (_globalId == 92){
            int dsdgdfgfd= 5;
        }
    }


    bool IdentBase::get_ident_is_finalize() const {
        return _isFinalize;
    }

    void IdentBase::set_ident_is_finalize() {
        IdentBase::_isFinalize = true;
    }

    const std::string &IdentBase::get_global_name() const {
        return _globalName;
    }

    void IdentBase::set_global_name(const std::string &global_name) {
        _globalName = global_name;
    }

    const std::vector<std::string> &IdentBase::get_inherit_name() const {
        return _inheritName;
    }

    void IdentBase::set_inherit_name(const std::vector<std::string> &inherit_name) {
        _inheritName = inherit_name;
    }

    std::string IdentBase::concat_inheritName() {
        assert(_isFinalize);
        std::string pre_ret;
        for (const auto& str: _inheritName){
            pre_ret += str + "_";
        }
        assert(pre_ret.size() > 1);
        return pre_ret.substr(0, pre_ret.size()-1);
    }

    IdentBase& IdentBase::operator = (const IdentBase& rhs){
        if (this == &rhs){
            return *this;
        }
        _isFinalize  = rhs._isFinalize;
        _globalId    = GLOBAL_MODEL_ID++;
        _globalName  = rhs._globalName + "_CP";
        _inheritName = rhs._inheritName;
        _inheritName.emplace_back("_CP");
        return *this;
    }

}