//
// Created by tanawin on 30/11/2566.
//

#include "controller.h"


namespace kathryn{

    /** central initializer*/
    ModelController*    centralControllerPtr = nullptr;
    ///Module*             globalModulePtr      = nullptr;


    ModelController::ModelController() {
        /** to prevent loop allocation in global module constructor*/
        centralControllerPtr = this;
        on_globalModule_init_component();
    }

    void ModelController::reset(){
        assert(!moduleStack.empty());
        /** delete old global module**/
        assert(moduleStack.size() == 1);
        on_module_final(moduleStack.top().md);
        delete globalModulePtr;
        globalModulePtr = nullptr;
        /** create new one*/
        on_globalModule_init_component();
    }

    Module* ModelController::getGlobalModule(){
        assert(globalModulePtr != nullptr);
        return globalModulePtr;
    }

    ModelController* getControllerPtr(){
        /// initiate controller before return
        /***lazy initializer*/
        if (centralControllerPtr == nullptr){
            new ModelController();
            /** the constructor of model controller will handle itself*/
        }else{
            return centralControllerPtr;
        }
        return centralControllerPtr;
    }

    Module* getGlobalModulePtr(){
        return getControllerPtr()->getGlobalModule();
    }

//    void freeControllerPtr(){
//        ///// finalize global module if it have
//        if (centralControllerPtr != nullptr){
//            centralControllerPtr->on_module_final(globalModulePtr);
//        }
//    }



}