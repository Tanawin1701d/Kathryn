//
// Created by tanawin on 28/11/2566.
//

#include "identifiable.h"
#include "model/hwComponent/module/module.h"


namespace kathryn{
    ull LAST_IDENT_ID = 0;

    void Identifiable::buildInheritName(){
        if (_parent != nullptr){
            _inheritName = _parent->getInheritName();
        }
        _inheritName.push_back(_globalName);
    }


    std::string Identifiable::concat_inheritName(){
        std::string preRet;
        for (const auto& str: _inheritName){
            preRet += str + "_";
        }
        assert(preRet.size() > 1);
        return preRet.substr(0, preRet.size()-1);


    }




}