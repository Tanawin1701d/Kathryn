//
// Created by tanawin on 30/11/2566.
//

#include "controller.h"


namespace kathryn{

    /** central initializer*/
    ModelController*    centralControllerPtr = nullptr;
    Module*        globalModulePtr = nullptr;

    ModelController* getControllerPtr(){
        /// initiate controller before return
        /***lazy initializer*/
        if (centralControllerPtr == nullptr){
            centralControllerPtr = new ModelController();
        }else{
            return centralControllerPtr;
        }
        /// if global module was not init, then init it
        if (globalModulePtr == nullptr){
            centralControllerPtr->unlockAllocation();
            globalModulePtr = new Module(false);
            globalModulePtr->setVarName("globeMod");
            centralControllerPtr->on_globalModule_init_component(globalModulePtr);
            initiateGlobalComponent();
        }

        return centralControllerPtr;
    }

    Module* getGlobalModulePtr(){
        if (globalModulePtr == nullptr){
            assert(centralControllerPtr == nullptr);
            getControllerPtr();
        }
        assert(globalModulePtr != nullptr);
        return globalModulePtr;
    }

    void freeControllerPtr(){
        ///// finalize global module if it have
        if (centralControllerPtr != nullptr){
            centralControllerPtr->on_module_final(globalModulePtr);
        }
    }

}