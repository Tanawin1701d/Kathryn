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
    void ModelController::on_sp_reg_init(CtrlFlowRegBase* ptr, SP_REG_TYPE spRegType) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /**localize necessary destination*/
        targetModulePtr->addSpReg(ptr, spRegType);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName(); //// build inherit name for that module
        if (ptr->requireResetEvent())
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

    void ModelController::on_reg_update(AsmNode* asmNode, Reg* srcReg){
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        tryPurifyFlowStack();
        asmNode->setDependStateJoinOp(BITWISE_AND);
        assert(isFlowBlockBaseofModuleInStack());
        auto fb = getTopFlowBlockBase();
        fb->addElementInFlowBlock(asmNode);
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

    void ModelController::on_wire_update(AsmNode* asmNode, Wire* srcWire) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        tryPurifyFlowStack();
        asmNode->setDependStateJoinOp(BITWISE_AND);
        //assert(!flowBlockStack.empty());
        if (isFlowBlockBaseofModuleInStack()) {
            /**in flow block*/
            auto fb = getTopFlowBlockBase();
            fb->addElementInFlowBlock(asmNode);
            logMF(srcWire,
                  "user wire is updating @ fb " + fb->getMdIdentVal());
        }else{
            asmNode->dryAssign();
            logMF(srcWire,
                  "user wire is updatting without flowblock");
            Module* targetModulePtr = getTargetModulePtr();
            targetModulePtr->addAsmNode(asmNode);
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

    /** memBlock*/
    void ModelController::on_memBlk_init(MemBlock* ptr){
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize it*/
        /** localize necessary destination*/
        targetModulePtr->addUserMemBlk(ptr);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        /** debug value*/
        logMF(ptr,
              "memBlk is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void ModelController::on_memBlkEleHolder_update(AsmNode* asmNode,MemBlockEleHolder* srcHolder){
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        tryPurifyFlowStack();
        asmNode->setDependStateJoinOp(BITWISE_AND);
        assert(isFlowBlockBaseofModuleInStack());
        auto fb = getTopFlowBlockBase();
        fb->addElementInFlowBlock(asmNode);
        logMF(srcHolder,
              "memBlk HOLDER is updating value @ fb block " + fb->getMdIdentVal());
    }

    /** exprMetas*/
    void ModelController::on_nest_init(nest* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTargetModulePtr();
        /** localize necessary destination*/
        targetModulePtr->addUserNest(ptr);
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        /** debug value*/
        logMF(ptr,
              "nest is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void ModelController::on_nest_update(AsmNode* asmNode, nest* srcWire) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        tryPurifyFlowStack();
        asmNode->setDependStateJoinOp(BITWISE_AND);
        //assert(!flowBlockStack.empty());
        if (isFlowBlockBaseofModuleInStack()) {
            /**in flow block*/
            auto fb = getTopFlowBlockBase();
            fb->addElementInFlowBlock(asmNode);
            logMF(srcWire,
                  "user nest is updating @ fb " + fb->getMdIdentVal());
        }else{
            asmNode->dryAssign();
            logMF(srcWire,
                  "user nest is updating without flowblock");
            Module* targetModulePtr = getTargetModulePtr();
            targetModulePtr->addAsmNode(asmNode);
        }
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
     *
     *
     * module
     *
     *
     *
     * */


    void ModelController::on_globalModule_init_component() {
        /**initiate component*/
        unlockAllocation();
        globalModulePtr = new Module(false);
        globalModulePtr->setVarName("globeMod");
        /** for global module for initialize project*/
        assert(globalModulePtr != nullptr);
        moduleStack.push(Module_Stack_Element{globalModulePtr, MODULE_COMPONENT_CONSTRUCT});
        globalModulePtr->buildInheritName();
        initiateGlobalComponent();
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