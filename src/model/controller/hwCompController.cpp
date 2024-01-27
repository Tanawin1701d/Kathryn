//
// Created by tanawin on 3/12/2566.
//

#include "controller.h"

#include "util/logger/logger.h"

namespace kathryn{


    Module* ModelController::getTargetModulePtr() {
        assert(!moduleStack.empty());
        /** base line must me */
        return moduleStack.top().md;
    }

    Module_Stack_Element& ModelController::getTargetModuleEle() {
        assert(!moduleStack.empty());
        return moduleStack.top();
    }


    /**
     * state register handling
     *
     * */
    void ModelController::on_sp_reg_init(Reg* ptr, SP_REG_TYPE spRegType) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /**localize necessary destination*/
        targetModulePtr->addSpReg(ptr, spRegType);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName(); //// build inherit name for that module
        ptr->makeResetEvent();
        /** debug value*/
        logMF(ptr,
              "[" + sp_reg_type_to_str(spRegType) + "] is initialized and set parent to "
              + targetModulePtr->getIdentDebugValue()
        );

    }

    /** register handling*/
    void ModelController::on_reg_init(Reg* ptr) {
        assert(ptr != nullptr);
        /** assign reg to module */
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserReg(ptr);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        ptr->makeResetEvent();
        /** debug value*/
        logMF(ptr,
              "USER_REG is initialized and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void ModelController::on_reg_update(AssignMeta* asmMeta, Reg* srcReg){
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmMeta != nullptr);
        purifyFlowStack();
        auto node = new AsmNode(asmMeta);
        assert(!flowBlockStack.empty());
        auto fb = flowBlockStack.top();
        fb->addElementInFlowBlock(node);
        logMF(srcReg,
              "user Reg is updating value @ fb block " + fb->getMdIdentVal());
    }


    /**
     *
     * wire handling
     *
     * */
    void ModelController::on_wire_init(Wire* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserWires(ptr);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        ptr->makeDefEvent();
        /** debug value*/
        logMF(ptr,
              "user wire is initialized and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void ModelController::on_wire_update(AssignMeta* asmMeta, Wire* srcWire) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmMeta != nullptr);
        purifyFlowStack();
        auto node = new AsmNode(asmMeta);
        //assert(!flowBlockStack.empty());
        if (!flowBlockStack.empty()) {
            /**in flow block*/
            auto fb = flowBlockStack.top();
            fb->addElementInFlowBlock(node);
            logMF(srcWire,
                  "user wire is updating @ fb " + fb->getMdIdentVal());
        }else{
            asmMeta->updateEventsPool.push_back(new UpdateEvent({
                nullptr,
                nullptr,
                &asmMeta->valueToAssign,
                asmMeta->desSlice,
                DEFAULT_UE_PRI_USER
            }));
            logMF(srcWire,
                  "user wire is updatting without flowblock");
        }
    }

    /** exprMetas*/
    void ModelController::on_expression_init(expression* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserExpression(ptr);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        /** debug value*/
        logMF(ptr,
              "expr is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }
    /** value*/
    void ModelController::on_value_init(Val* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserVal(ptr);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        /** debug value*/
        logMF(ptr,
              "val is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    /**
     * module
     * */


    void ModelController::on_globalModule_init_component(Module* globalMod) {
        /** for global module for initialize project*/
        assert(globalMod != nullptr);
        moduleStack.push(Module_Stack_Element{globalMod, MODULE_COMPONENT_CONSTRUCT});
        globalMod->buildInheritName();
    }


    void ModelController::on_module_init_components(Module* ptr) {
        /**check that module initialization is in construct state not in designflow constructing*/
        assert(getTargetModuleEle().state == MODULE_COMPONENT_CONSTRUCT);
        /** previous module*/
        Module* targetModulePtr = getTargetModulePtr();
        targetModulePtr->addUserSubModule(ptr);
        /** current module*/
        /**at least module must be other submodule*/
        assert(getTargetModuleEle().state != MODULE_FINISHED_CONSTRUCT);
        moduleStack.push(Module_Stack_Element{ptr, MODULE_COMPONENT_CONSTRUCT});
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        /** debug value*/
        logMF(ptr,
              "module is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void ModelController::on_module_init_designFlow(Module* ptr) {
        Module* topModule = getTargetModulePtr();
        assert(topModule == ptr);
        moduleStack.top().state = MODULE_DESIGN_FLOW_CONSTRUCT;
        /** debug value*/
        logMF(ptr,
              "module is initializing design flow");
        /** flow the program*/
        topModule->flow();
        topModule->buildFlow();

    }

    void ModelController::on_module_final(Module* ptr) {
        assert(!moduleStack.empty() && ( moduleStack.top().md == ptr ));
        moduleStack.top().state = MODULE_FINISHED_CONSTRUCT;
        assert(isAllFlowStackEmpty());
        moduleStack.pop();
        logMF(ptr,
              "module is finalized design flow");
    }


}