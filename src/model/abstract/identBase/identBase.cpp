//
// Created by tanawin on 29/3/2567.
//

#include "identBase.h"

namespace kathryn{


    ull GLOBAL_MODEL_ID = 0;

    ull getLastIdentId(){
        return GLOBAL_MODEL_ID;
    }


    IdentBase::IdentBase() {

        _globalId = GLOBAL_MODEL_ID++;

        if (_globalId == 46){
            int dsdgdfgfd= 5;
        }
    }


    bool IdentBase::getIdentIsFinalize() const {
        return _isFinalize;
    }

    void IdentBase::setIdentIsFinalize() {
        IdentBase::_isFinalize = true;
    }

    const std::string &IdentBase::getGlobalName() const {
        return _globalName;
    }

    void IdentBase::setGlobalName(const std::string &globalName) {
        _globalName = globalName;
    }

    const std::vector<std::string> &IdentBase::getInheritName() const {
        return _inheritName;
    }

    void IdentBase::setInheritName(const std::vector<std::string> &inheritName) {
        _inheritName = inheritName;
    }

    std::string IdentBase::concat_inheritName() {
        assert(_isFinalize);
        std::string preRet;
        for (const auto& str: _inheritName){
            preRet += str + "_";
        }
        assert(preRet.size() > 1);
        return preRet.substr(0, preRet.size()-1);
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