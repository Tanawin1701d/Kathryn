//
// Created by tanawin on 3/12/2566.
//

#include "controller.h"
#include "model/FlowBlock/abstract/flowBlock_Base.h"

namespace kathryn{


    ModulePtr Controller::getTargetModulePtr() {
        /** base line must me */
        return moduleStack.top().md;
    }

    /** register handling*/
    void Controller::on_reg_init(const RegPtr& ptr) {
        /** assign reg to module */
        ModulePtr targetModule = getTargetModulePtr();
        targetModule->addUserReg(ptr);

    }

    void Controller::on_reg_update(const std::shared_ptr<UpdateEvent>& upEvent){
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        auto node = std::make_shared<Node>(upEvent);
        assert(!flowBlockStack.empty());
        auto fb = flowBlockStack.top();
        fb->addElementInFlowBlock(node);
    }

    /** wire handling*/
    void Controller::on_wire_init(const WirePtr& ptr) {
        ModulePtr targetModule = getTargetModulePtr();
        targetModule->addUserWires(ptr);
    }

    void Controller::on_wire_update(const std::shared_ptr<UpdateEvent>& upEvent) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        auto node = std::make_shared<Node>(upEvent);
        assert(!flowBlockStack.empty());
        auto fb = flowBlockStack.top();
        fb->addElementInFlowBlock(node);
    }

    /** expression*/
    void Controller::on_expression_init(const expressionPtr& ptr) {
        ModulePtr targetModule = getTargetModulePtr();
        targetModule->addUserExpression(ptr);
    }
    /** value*/
    void Controller::on_value_init(const ValPtr &ptr) {
        ModulePtr targetModule = getTargetModulePtr();
        targetModule->addUserVal(ptr);
    }

    /**
     * module
     * */
    void Controller::on_module_init_components(const ModulePtr& ptr) {
        /** previous module*/
        ModulePtr targetModule = getTargetModulePtr();
        targetModule->addUserSubModule(ptr);
        /** current module*/
        /**at least module must be other submodule*/
        assert(moduleStack.top().state != MODULE_FINISHED_CONSTRUCT);
        moduleStack.push(Module_Stack_Element{ptr, MODULE_COMPONENT_CONSTRUCT});

    }


    void Controller::on_module_init_designFlow(ModulePtr ptr) {
        ModulePtr topModule = getTargetModulePtr();
        assert(topModule == ptr);
        moduleStack.top().state = MODULE_DESIGN_FLOW_CONSTRUCT;
        /** flow the program*/
        topModule->flow();

    }


    void Controller::on_module_final(ModulePtr ptr) {
        assert(!moduleStack.empty() && ( moduleStack.top().md == ptr ));
        moduleStack.top().state = MODULE_FINISHED_CONSTRUCT;
        assert(isAllFlowStackEmpty());
        moduleStack.pop();
    }


}