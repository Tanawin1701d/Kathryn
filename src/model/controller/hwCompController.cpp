//
// Created by tanawin on 3/12/2566.
//

#include "controller.h"

#include "util/logger/logger.h"

namespace kathryn{


    Module* ModelController::getTopModulePtr() {
        /** base line must be auto initialized */
        assert(!moduleStack.empty());
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
        Module* targetModulePtr = getTopModulePtr();
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
        Module* targetModulePtr = getTopModulePtr();
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
        if(isTopFbBelongToTopModule()){
            auto fb = getTopFlowBlockBase();
            fb->addElementInFlowBlock(asmNode);
            logMF(srcReg,
                  "user Reg is updating value @ fb block " + fb->getMdIdentVal());
        }else{
            asmNode->dryAssign();
            logMF(srcReg,
                  "user reg is updatting without flowblock");
            Module* targetModulePtr = getTopModulePtr();
            targetModulePtr->addNode(asmNode);
        }

    }


    /**
     *
     * wire handling
     *
     * */
    void ModelController::on_wire_init(Wire* ptr){
        assert(ptr != nullptr);
        Module* targetModulePtr = getTopModulePtr();
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
        if (isTopFbBelongToTopModule()) {
            /**in flow block*/
            auto fb = getTopFlowBlockBase();
            fb->addElementInFlowBlock(asmNode);
            logMF(srcWire,
                  "user wire is updating @ fb " + fb->getMdIdentVal());
        }else{
            asmNode->dryAssign();
            logMF(srcWire,
                  "user wire is updatting without flowblock");
            Module* targetModulePtr = getTopModulePtr();
            targetModulePtr->addNode(asmNode);
        }
    }

    /** exprMetas*/
    void ModelController::on_expression_init(expression* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTopModulePtr();
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
        Module* targetModulePtr = getTopModulePtr();
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
        if(isTopFbBelongToTopModule()){
            auto fb = getTopFlowBlockBase();
            fb->addElementInFlowBlock(asmNode);
            logMF(srcHolder,
                  "memBlk HOLDER is updating value @ fb block " + fb->getMdIdentVal());
        }else{
            asmNode->dryAssign();
            logMF(srcHolder,
                  "user mem holder is updatting without flowblock");
            Module* targetModulePtr = getTopModulePtr();
            targetModulePtr->addNode(asmNode);
        }

    }

    /** exprMetas*/
    void ModelController::on_nest_init(nest* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTopModulePtr();
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
        if (isTopFbBelongToTopModule()) {
            /**in flow block*/
            auto fb = getTopFlowBlockBase();
            fb->addElementInFlowBlock(asmNode);
            logMF(srcWire,
                  "user nest is updating @ fb " + fb->getMdIdentVal());
        }else{
            asmNode->dryAssign();
            logMF(srcWire,
                  "user nest is updating without flowblock");
            Module* targetModulePtr = getTopModulePtr();
            targetModulePtr->addNode(asmNode);
        }
    }


    /** value*/
    void ModelController::on_value_init(Val* ptr) {
        assert(ptr != nullptr);
        Module* targetModulePtr = getTopModulePtr();
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
        moduleStack.push(Module_Stack_Element{globalModulePtr, MODULE_INIT});
        globalModulePtr->buildInheritName();
        initiateGlobalComponent();
    }

    void ModelController::on_globalModule_init_designFlow(){
        assert(globalModulePtr != nullptr);
        assert(moduleStack.empty());
        assert(isAllFlowStackEmpty());
        moduleStack.push(Module_Stack_Element{globalModulePtr, MODULE_INIT_DESIGN_FLOW});
        globalModulePtr->buildAll();
    }


    void ModelController::on_module_init_components(Module* ptr) {
        /**check that module initialization is in construct state not in designflow constructing*/
        assert(getTargetModuleEle().state == MODULE_INIT || getTargetModuleEle().state == MODULE_INIT_DESIGN_FLOW);
        assert(ptr->getStage() == MODEL_UNINIT);
        /** previous module*/
        Module* targetModulePtr = getTopModulePtr();
        targetModulePtr->addUserSubModule(ptr);
        /** current module*/
        /**at least module must be other submodule*/
        moduleStack.push(Module_Stack_Element{ptr, MODULE_INIT});
        ptr->setParent(targetModulePtr);
        ptr->buildInheritName();
        /** debug value*/
        logMF(ptr,
              "module is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void ModelController::on_module_end_init_components(Module* ptr){
        assert(!moduleStack.empty() && (moduleStack.top().md == ptr));
        assert(moduleStack.top().state == MODULE_INIT);
        ptr->setStage(MODEL_GLOB_INITED);
        moduleStack.top().state = MODULE_END_GLOB_DECLARE;
        moduleStack.pop();

        logMF(ptr, "module is finalize init component");
    }



    void ModelController::on_module_init_designFlow(Module* ptr) {
        assert(ptr->getStage() == MODEL_GLOB_INITED);
        assert(getTargetModuleEle().state == MODULE_INIT_DESIGN_FLOW);
        assert(ptr != nullptr);
        /** push to model stack */
        moduleStack.push(Module_Stack_Element{ptr, MODULE_INIT_DESIGN_FLOW});
        /** debug value*/
        logMF(ptr,
              "module is initializing design flow");
        /** flow the program*/
        ptr->buildAll();

    }

    void ModelController::on_module_final(Module* ptr) {
        assert(!moduleStack.empty() && ( moduleStack.top().md == ptr ));
        assert(moduleStack.top().state == MODULE_INIT_DESIGN_FLOW);
        ptr->setStage(MODEL_FLOW_INITED);
        moduleStack.top().state = MODULE_END;
        assert(!isTopFbBelongToTopModule());
        moduleStack.pop();
        logMF(ptr,
              "module is finalized design flow");
    }

}