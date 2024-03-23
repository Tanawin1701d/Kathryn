//
// Created by tanawin on 28/11/2566.
//

#include "identifiable.h"
#include "model/hwComponent/module/module.h"


namespace kathryn{
    ull LAST_IDENT_ID = 0;

    ull getLastIdentId(){
        return LAST_IDENT_ID;
    }

    /***
     *
     *
     * var name management colab with make component
     *
     * */

    ull  lastIdent = -1;
    std::string varName = "UNNAME";

    bool isVarNameRetrievable(ull deviceIdentId){
        return lastIdent == deviceIdentId;
    }

    std::string retrieveVarName(){
        lastIdent = -1;
        std::string preRet = "UNNAME";
        swap(varName, preRet);
        return preRet;
    }

    void setRetrieveVarName(std::string name){
        lastIdent    = getLastIdentId();
        varName      = std::move(name);
    }



    /**
     *
     *
     *
     *
     * */




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