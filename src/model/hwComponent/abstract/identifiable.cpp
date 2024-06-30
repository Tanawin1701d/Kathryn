//
// Created by tanawin on 28/11/2566.
//

#include "identifiable.h"

#include <utility>
#include "model/hwComponent/module/module.h"


namespace kathryn{

    /***
     *
     *
     * var name management colab with make component
     *
     * */



    ull  lastIdent = -1;
    const VarMeta VARMETADEF = {"UN_INIT_VARMETA_TYPE","UN_INIT_VARMETA_TYPE", false};
    VarMeta varMeta = VARMETADEF;

    bool isVarNameRetrievable(ull deviceIdentId){
        return lastIdent == deviceIdentId;
    }

    VarMeta retrieveVarMeta(){
        lastIdent = -1;
        VarMeta preRet = varMeta;
        varMeta = VARMETADEF;
        return preRet;
    }

    void setRetrieveVarMeta(std::string varType,std::string name, bool isUserDec){
        lastIdent    = getLastIdentId();
        varMeta      = {std::move(varType), std::move(name), isUserDec};
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