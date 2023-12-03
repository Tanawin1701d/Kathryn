//
// Created by tanawin on 30/11/2566.
//

#include "controller.h"


namespace kathryn{

    /** central initializer*/
    ControllerPtr    centralControllerPtr = nullptr;
    ModulePtr        globalModulePtr = nullptr;

    ControllerPtr getControllerPtr(){
        /// initiate controller before return
        /***lazy initializer*/
        if (centralControllerPtr == nullptr){
            centralControllerPtr = std::make_shared<Controller>();
        }
        /// if global module was not init, then init it
        if (globalModulePtr == nullptr){
            globalModulePtr = std::make_shared<Module>();
            centralControllerPtr->on_module_init_components(globalModulePtr);
        }
        return centralControllerPtr;
    }

    void freeControllerPtr(){
        ///// finalize global module if it have
        if (centralControllerPtr != nullptr){
            centralControllerPtr->on_module_final(globalModulePtr);
        }
    }

}