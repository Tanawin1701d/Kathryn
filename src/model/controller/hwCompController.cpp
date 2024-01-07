//
// Created by tanawin on 3/12/2566.
//

#include "controller.h"

#include "util/logger/logger.h"

namespace kathryn{


    Module* Controller::getTargetModulePtr() {
        assert(!moduleStack.empty());
        /** base line must me */
        return moduleStack.top().md;
    }

    Module_Stack_Element& Controller::getTargetModuleEle() {
        assert(!moduleStack.empty());
        return moduleStack.top();
    }


    /**
     * state register handling
     *
     * */
    void Controller::on_state_reg_init(StateReg* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /**localize necessary destination*/
        targetModulePtr->addStateReg(ptr);
        ptr->setParent(targetModulePtr);
        /** debug value*/
        logMF("STATE_REG " + ptr->getIdentDebugValue(),
              "stateReg is initialized and set parent to " + targetModulePtr->getIdentDebugValue());

    }


    void Controller::on_cond_wait_reg_init(CondWaitStateReg* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /**localize necessary destination*/
        targetModulePtr->addCondWaitStateReg(ptr);
        ptr->setParent(targetModulePtr);
        /** debug value*/
        logMF("CONDWAIT_REG " + ptr->getIdentDebugValue(),
              "stateReg is initialized and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void Controller::on_cycle_wait_reg_init(CycleWaitStateReg* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /**localize necessary destination*/
        targetModulePtr->addCycleWaitStateReg(ptr);
        ptr->setParent(targetModulePtr);
        /** debug value*/
        logMF("CYWAIT_REG " + ptr->getIdentDebugValue(),
              "stateReg is initialized and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    /** register handling*/
    void Controller::on_reg_init(Reg* ptr) {
        assert(ptr != nullptr);
        /** assign reg to module */
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserReg(ptr);
        ptr->setParent(targetModulePtr);
        /** debug value*/
        logMF("USER_REG " + ptr->getIdentDebugValue(),
              "user Reg is initialized and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void Controller::on_reg_update(AssignMeta* asmMeta, Reg* srcReg){
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
        logMF("USER_REG " + srcReg->getIdentDebugValue(),
              "user Reg is updating value @ fb block " + fb->getFlowBlockDebugIdentValue());
    }


    /**
     *
     * wire handling
     *
     * */
    void Controller::on_wire_init(Wire* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserWires(ptr);
        ptr->setParent(targetModulePtr);
        /** debug value*/
        logMF("USER_Wire " + ptr->getIdentDebugValue(),
              "user wire is initialized and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void Controller::on_wire_update(AssignMeta* asmMeta, Wire* srcWire) {
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
        logMF("USER_Wire " + srcWire->getIdentDebugValue(),
              "user wire is updating @ fb " + fb->getFlowBlockDebugIdentValue());
    }

    /** exprMetas*/
    void Controller::on_expression_init(expression* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserExpression(ptr);
        ptr->setParent(targetModulePtr);
        /** debug value*/
        logMF("Expr " + ptr->getIdentDebugValue(),
              "expr is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }
    /** value*/
    void Controller::on_value_init(Val* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserVal(ptr);
        ptr->setParent(targetModulePtr);
        /** debug value*/
        logMF("Val " + ptr->getIdentDebugValue(),
              "val is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    /**
     * module
     * */


    void Controller::on_globalModule_init_component(Module* globalMod) {
        /** for global module for initialize project*/
        assert(globalMod != nullptr);
        moduleStack.push(Module_Stack_Element{globalMod, MODULE_COMPONENT_CONSTRUCT});
    }


    void Controller::on_module_init_components(Module* ptr) {
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
        /** debug value*/
        logMF("USER_MODULE " + ptr->getIdentDebugValue(),
              "module is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void Controller::on_module_init_designFlow(Module* ptr) {
        Module* topModule = getTargetModulePtr();
        assert(topModule == ptr);
        moduleStack.top().state = MODULE_DESIGN_FLOW_CONSTRUCT;
        /** debug value*/
        logMF("USER_MODULE " + ptr->getIdentDebugValue(),
              "module is initializing design flow");
        /** flow the program*/
        topModule->flow();
        topModule->buildFlow();

    }

    void Controller::on_module_final(Module* ptr) {
        assert(!moduleStack.empty() && ( moduleStack.top().md == ptr ));
        moduleStack.top().state = MODULE_FINISHED_CONSTRUCT;
        assert(isAllFlowStackEmpty());
        moduleStack.pop();
        logMF("USER_MODULE " + ptr->getIdentDebugValue(),
              "module is finalized design flow");
    }


}