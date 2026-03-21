//
// Created by tanawin on 3/10/2024.
//

#ifndef src_model_interface_base_INTERFACE_H
#define src_model_interface_base_INTERFACE_H
#include "model/hw_component/abstract/identifiable.h"

#include "model/controller/con_interf/controller_itf.h"

namespace kathryn{

    class ModelInterface: public HwCompControllerItf,
                          public Identifiable,
                          public ModelDebuggable
        {
    public:


        ModelInterface():
        HwCompControllerItf(false),
        Identifiable(TYPE_INTF){
            com_init();
        }

        void com_init() override;

        virtual void build_logic_base() = 0;
        virtual void build_user_logic(){ };
        virtual void transfer_pay_load() = 0;


        //////// model debugable

        std::string get_md_ident_val() override{
            return get_ident_debug_value();
        }

    };

}

#endif //src_model_interface_base_INTERFACE_H
