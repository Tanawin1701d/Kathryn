//
// Created by tanawin on 3/12/2566.
//

#include "controller.h"
#include "node.h"

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

    /** register handling*/
    void Controller::on_reg_init(Reg* ptr) {
        assert(ptr != nullptr);
        /** assign reg to module */
        Module* targetModule = getTargetModulePtr();
        /** localize necessary destination*/
        targetModule->addUserReg(ptr);
        ptr->setParent(targetModule);
    }

    void Controller::on_reg_update(AssignMeta* asmMeta){
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmMeta != nullptr);
        purifyFlowStack();
        auto node = new Node(asmMeta);
        assert(!flowBlockStack.empty());
        auto fb = flowBlockStack.top();
        fb->addElementInFlowBlock(node);
    }

    /** state register handling */
    void Controller::on_state_reg_init(StateReg* ptr) {
        assert(ptr != nullptr);
        Module* targetModule = getTargetModulePtr();
        /**localize necessary destination*/
        targetModule->addStateReg(ptr);
        ptr->setParent(targetModule);
    }

    /** wire handling*/
    void Controller::on_wire_init(Wire* ptr) {
        assert(ptr != nullptr);
        Module* targetModule = getTargetModulePtr();
        /** localize necessary destination*/
        targetModule->addUserWires(ptr);
        ptr->setParent(targetModule);
    }

    void Controller::on_wire_update(AssignMeta* asmMeta) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmMeta != nullptr);
        purifyFlowStack();
        auto node = new Node(asmMeta);
        assert(!flowBlockStack.empty());
        auto fb = flowBlockStack.top();
        fb->addElementInFlowBlock(node);
    }

    /** exprMetas*/
    void Controller::on_expression_init(expression* ptr) {
        assert(ptr != nullptr);
        Module* targetModule = getTargetModulePtr();
        /** localize necessary destination*/
        targetModule->addUserExpression(ptr);
        ptr->setParent(targetModule);
    }
    /** value*/
    void Controller::on_value_init(Val* ptr) {
        assert(ptr != nullptr);
        Module* targetModule = getTargetModulePtr();
        /** localize necessary destination*/
        targetModule->addUserVal(ptr);
        ptr->setParent(targetModule);
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
        Module* targetModule = getTargetModulePtr();
        targetModule->addUserSubModule(ptr);
        /** current module*/
        /**at least module must be other submodule*/
        assert(getTargetModuleEle().state != MODULE_FINISHED_CONSTRUCT);
        moduleStack.push(Module_Stack_Element{ptr, MODULE_COMPONENT_CONSTRUCT});
        ptr->setParent(targetModule);

    }

    void Controller::on_module_init_designFlow(Module* ptr) {
        Module* topModule = getTargetModulePtr();
        assert(topModule == ptr);
        moduleStack.top().state = MODULE_DESIGN_FLOW_CONSTRUCT;
        /** flow the program*/
        topModule->flow();
        topModule->buildFlow();
    }

    void Controller::on_module_final(Module* ptr) {
        assert(!moduleStack.empty() && ( moduleStack.top().md == ptr ));
        moduleStack.top().state = MODULE_FINISHED_CONSTRUCT;
        assert(isAllFlowStackEmpty());
        moduleStack.pop();
    }


}