//
// Created by tanawin on 3/10/2024.
//

#ifndef src_model_interface_base_INTERFACE_H
#define src_model_interface_base_INTERFACE_H
#include "model/hwComponent/abstract/identifiable.h"

#include "model/controller/conInterf/controllerItf.h"

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

        virtual void buildLogicBase() = 0;
        virtual void buildUserLogic(){ };
        virtual void transferPayLoad() = 0;


        //////// model debugable

        std::string getMdIdentVal() override{
            return getIdentDebugValue();
        }

    };

}

#endif //src_model_interface_base_INTERFACE_H
