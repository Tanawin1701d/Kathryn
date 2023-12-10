//
// Created by tanawin on 30/11/2566.
//

#include "controller.h"


namespace kathryn{

    /** central initializer*/
    Controller*    centralControllerPtr = nullptr;
    Module*        globalModulePtr = nullptr;

    Controller* getControllerPtr(){
        /// initiate controller before return
        /***lazy initializer*/
        if (centralControllerPtr == nullptr){
            centralControllerPtr = new Controller();
        }
        /// if global module was not init, then init it
        if (globalModulePtr == nullptr){
            globalModulePtr = new Module();
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