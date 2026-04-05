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
        lastIdent    = get_last_ident_id();
        varMeta      = {std::move(varType), std::move(name), isUserDec};
    }



    /**
     *
     *
     *
     *
     * */

    void Identifiable::build_inherit_name(){
        if (_parent != nullptr){
            _inherit_name = _parent->get_inherit_name();
        }
        _inherit_name.push_back(_global_name);
        finalize_ident();
    }




}