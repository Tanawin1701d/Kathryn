//
// Created by tanawin on 28/11/2566.
//

#include "identifiable.h"

#include "utility"
#include "model/hw_component/module/module.h"


namespace kathryn{

    /***
     *
     *
     * var name management colab with make component
     *
     * */



    ull  last_ident = -1;
    const VarMeta VARMETADEF = {"UN_INIT_VARMETA_TYPE","UN_INIT_VARMETA_TYPE", false};
    VarMeta var_meta = VARMETADEF;

    bool is_var_name_retrievable(ull device_ident_id){
        return last_ident == device_ident_id;
    }

    VarMeta retrieve_var_meta(){
        last_ident = -1;
        VarMeta pre_ret = var_meta;
        var_meta = VARMETADEF;
        return pre_ret;
    }

    void set_retrieve_var_meta(std::string var_type,std::string name, bool is_user_dec){
        last_ident    = get_last_ident_id();
        var_meta      = {std::move(var_type), std::move(name), is_user_dec};
    }



    /**
     *
     *
     *
     *
     * */

    void Identifiable::build_inherit_name(){
        if (_parent != nullptr){
            _inheritName = _parent->get_inherit_name();
        }
        _inheritName.push_back(_globalName);
        set_ident_is_finalize();
    }




}