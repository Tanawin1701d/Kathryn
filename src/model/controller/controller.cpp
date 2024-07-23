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

    void ModelController::start(){
        assert(globalModulePtr != nullptr);
        pipeCtrl.start();
        /***
         * global Module must be auto initiated when controller is initialize or it is reset
         * */
        on_module_end_init_components(globalModulePtr);
        on_globalModule_init_designFlow();
        on_module_final(moduleStack.top().md);
    }


    void ModelController::reset(){
        pipeCtrl.reset();
        clean();
        on_globalModule_init_component();
    }

    void ModelController::clean(){
        /** delete old global module**/
        pipeCtrl.clean();
        assert(isAllFlowStackEmpty());
        assert(moduleStack.empty());
        delete globalModulePtr;
        globalModulePtr = nullptr;
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
        }
        return centralControllerPtr;
    }

    Module* getGlobalModulePtr(){
        return getControllerPtr()->getGlobalModule();
    }



    std::string ModelController::getCurModelStack() {

        std::vector<Module_Stack_Element> mdVec = cvtStackToVec(moduleStack);
        std::vector<FlowBlockBase*>       fbVec = cvtStackToVec(flowBlockStacks[FLOW_ST_BASE_STACK]);

        int accumIdent = 0;
        std::string result;

        for (auto mod: mdVec){
            result += (mod.md->getVarName() + "\n");
            result += genConString(' ', accumIdent);
            accumIdent += 4;
        }

        for (int i = 0; i < fbVec.size(); i++){
            std::string position;
            if (i > 0){
                if (fbVec[i]->getFlowType() != CSELIF && fbVec[i]->getFlowType() != CSELSE) {
                    position += " subBlockIdx " + std::to_string(fbVec[i - 1]->getSubBlocks().size()) + "    ";
                }
                position += " conBlockIdx " + std::to_string(fbVec[i-1]->getConBlocks().size());
            }
            result += (fbVec[i]->getGlobalName() + "@ " + position + "\n" );
            result += genConString(' ', accumIdent);
            accumIdent += 4;
        }

        return result;

    }



//    void freeControllerPtr(){
//        ///// finalize global module if it have
//        if (centralControllerPtr != nullptr){
//            centralControllerPtr->on_module_final(globalModulePtr);
//        }
//    }



}
