//
// Created by tanawin on 28/11/2566.
//

#include "identifiable.h"
#include "model/hwComponent/module/module.h"


namespace kathryn{

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
        setIdentIsFinalize();
    }




}