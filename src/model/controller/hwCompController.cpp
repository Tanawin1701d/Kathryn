//
// Created by tanawin on 3/12/2566.
//

#include "model/hwComponent/globalComponent/globalComponent.h"

#include "controller.h"

#include "util/logger/logger.h"

namespace kathryn{


    Module* ModelController::get_top_module_ptr() {
        /** base line must be auto initialized */
        assert(!_module_stack.empty());
        return _module_stack.top().md;
    }

    Module_Stack_Element& ModelController::get_target_module_ele() {
        assert(!_module_stack.empty());
        return _module_stack.top();
    }

    /**
     * state register handling
     *
     * */
    void ModelController::on_sp_reg_init(CtrlFlowRegBase* ptr, SP_REG_TYPE spRegType) {
        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /**localize necessary destination*/
        target_module_ptr->addSpReg(ptr, spRegType);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name(); //// build inherit name for that module
        if (ptr->requireResetEvent())
            ptr->makeResetEvent();
        /** debug value*/
        logMF(ptr,
              "[" + sp_reg_type_to_str(spRegType) + "] is initialized and set parent to "
              + target_module_ptr->getIdentDebugValue()
        );

    }

    /** register handling*/
    void ModelController::on_reg_init(Reg* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        /** assign reg to module */
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->addUserReg(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();
        ////ptr->makeResetEvent();
        /** debug value*/
        logMF(ptr,
              "USER_REG is initialized and set parent to " + target_module_ptr->getIdentDebugValue());
    }

    void ModelController::on_reg_update(AsmNode* asmNode, Reg* src_reg){
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        try_purify_flow_stack();
        if(is_top_fb_belong_to_top_module()){
            auto fb = get_top_flow_block_base_ptr();
            fb->add_basic_node(asmNode);
            logMF(src_reg,
                  "user Reg is updating value @ fb block " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asmNode);

        }

    }


    /**
     *
     * wire handling
     *
     * */
    void ModelController::on_wire_init(Wire* ptr){
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->addUserWires(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();
        ptr->makeDefEvent();

        /** debug value*/
        logMF(ptr,
              "user wire is initialized and set parent to " + target_module_ptr->getIdentDebugValue());
    }

    void ModelController::on_wire_update(AsmNode* asmNode, Wire* src_wire) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        try_purify_flow_stack();
        //assert(!flowBlockStack.empty());
        if (is_top_fb_belong_to_top_module()) {
            /**in flow block*/
            auto fb = get_top_flow_block_base_ptr();
            fb->add_basic_node(asmNode);
            logMF(src_wire,
                  "user wire is updating @ fb " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asmNode);
        }
    }

    /** exprMetas*/
    void ModelController::on_expression_init(expression* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->addUserExpression(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();

        /** debug value*/
        logMF(ptr,
              "expr is initializing and set parent to " + target_module_ptr->getIdentDebugValue());
    }

    /** memBlock*/
    void ModelController::on_memBlk_init(MemBlock* ptr){
        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize it*/
        /** localize necessary destination*/
        target_module_ptr->addUserMemBlk(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();
        /** debug value*/
        logMF(ptr,
              "memBlk is initializing and set parent to " + target_module_ptr->getIdentDebugValue());
    }

    void ModelController::on_memBlkEleHolder_update(AsmNode* asm_node,MemBlockEleHolder* src_holder){
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asm_node != nullptr);
        try_purify_flow_stack();
        if(is_top_fb_belong_to_top_module()){
            auto fb = get_top_flow_block_base_ptr();
            fb->add_basic_node(asm_node);
            logMF(src_holder,
                  "memBlk HOLDER is updating value @ fb block " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asm_node);
        }

    }

    /** nest*/
    void ModelController::on_nest_init(nest* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->addUserNest(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();

        /** debug value*/
        logMF(ptr,
              "nest is initializing and set parent to " + target_module_ptr->getIdentDebugValue());
    }

    void ModelController::on_nest_update(AsmNode* asmNode, nest* src_nest) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        try_purify_flow_stack();
        //assert(!flowBlockStack.empty());
        if (is_top_fb_belong_to_top_module()) {
            /**in flow block*/
            auto fb = get_top_flow_block_base_ptr();
            fb->add_basic_node(asmNode);
            logMF(src_nest,
                  "user nest is updating @ fb " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asmNode);
        }
    }


    /** value*/
    void ModelController::on_value_init(Val* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->addUserVal(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();
        /** debug value*/
        logMF(ptr,
              "val is initializing and set parent to " + target_module_ptr->getIdentDebugValue());
    }

    /** pmValue*/
    void ModelController::on_pmValue_init(PmVal* ptr) {
        assert(ptr != nullptr);
        assert(_box_stack.empty());
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->addUserPmVal(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();
        logMF(ptr, "pmval is initializing and set parent to " + target_module_ptr->getIdentDebugValue());

    }

    /** box*/
    void ModelController::on_box_init(Box* ptr) {
        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        ////// if it is slave block do not push it to module storage
        ptr->setParent(target_module_ptr);
        if (_box_stack.empty()){
            target_module_ptr->addUserBox(ptr);
        }else{
            Box* topBox = _box_stack.top();
            topBox->addSubBox(ptr);
        }
        ptr->build_inherit_name();

        _box_stack.push(ptr);
        /** debug value*/
        logMF(ptr, "box is initializing and set parent to " +
                   target_module_ptr->getIdentDebugValue());
    }

    void ModelController::on_box_end_init(Box* ptr){
        /**check that the stack is not do somthing wrong*/
        assert(ptr == _box_stack.top());
        _box_stack.pop();
        logMF(ptr, "box is pick out from stack");
    }

    void ModelController::on_box_update(AsmNode* asmNode, Box* srcBox) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asmNode != nullptr);
        try_purify_flow_stack();
        //assert(!flowBlockStack.empty());
        if (is_top_fb_belong_to_top_module()) {
            /**in flow block*/
            auto fb = get_top_flow_block_base_ptr();
            fb->add_basic_node(asmNode);
            logMF(srcBox,
                  "user nest is updating @ fb " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asmNode);
        }
    }

    void ModelController::on_box_tryAddToBox(Operable* opr1, Assignable* asb){
        assert(asb  != nullptr);
        assert(opr1 != nullptr);
        /**check that box is being fomation and make sure reg is from user */
        if ( (!_box_stack.empty()) && opr1->castToIdent()->isUserVar()){
            _box_stack.top()->addNestMeta({opr1, asb});
        }
    }


    void ModelController::on_itf_init(ModelInterface* ptr){

        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->addUserItf(ptr);
        ptr->setParent(target_module_ptr);
        ptr->build_inherit_name();
        /** debug value*/
        logMF(ptr,
              "val is initializing and set parent to " + target_module_ptr->getIdentDebugValue());

    }

    /**
     *
     *
     * module
     *
     *
     *
     * */

    ///////// global module

    void ModelController::on_globalModule_init_component() {
        /**initiate component*/
        unlock_allocation();
        _global_module_ptr = new Module(false);
        _global_module_ptr->setVarName("globeMod");
        /** for global module for initialize project*/
        assert(_global_module_ptr != nullptr);
        _module_stack.push(Module_Stack_Element{_global_module_ptr, MODULE_INIT});
        _global_module_ptr->build_inherit_name();
        initiateGlobalComponent();
    }

    void ModelController::on_globalModule_init_designFlow(){
        assert(_global_module_ptr != nullptr);
        assert(_module_stack.empty());
        assert(is_all_flow_stack_empty());
        _module_stack.push(Module_Stack_Element{_global_module_ptr, MODULE_INIT_DESIGN_FLOW});
        _global_module_ptr->buildAll();
    }

    void ModelController::on_globalModule_init_auxilaryComponent(){

        assert(_global_module_ptr != nullptr);
        assert(_module_stack.empty());
        assert(is_all_flow_stack_empty());
        _module_stack.push(Module_Stack_Element{_global_module_ptr, MODULE_INIT_AUX});

    }

    void ModelController::on_globalModule_final_auxilaryComponent(){

        assert(!_module_stack.empty());
        assert(_module_stack.top().state == MODULE_INIT_AUX);
        _module_stack.top().state = MODULE_INIT_AUX;
        _module_stack.pop();

    }








    ///////// normal module


    void ModelController::on_module_init_components(Module* ptr) {
        /**check that module initialization is in construct state not in designflow constructing*/
        assert(get_target_module_ele().state == MODULE_INIT || get_target_module_ele().state == MODULE_INIT_DESIGN_FLOW);
        assert(ptr->getStage() == MODEL_UNINIT);
        /** previous module*/
        Module* targetModulePtr = get_top_module_ptr();
        targetModulePtr->addUserSubModule(ptr);
        /** current module*/
        /**at least module must be other submodule*/
        _module_stack.push(Module_Stack_Element{ptr, MODULE_INIT});
        ptr->setParent(targetModulePtr);
        ptr->build_inherit_name();
        /** debug value*/
        logMF(ptr,
              "module is initializing and set parent to " + targetModulePtr->getIdentDebugValue());
    }

    void ModelController::on_module_end_init_components(Module* ptr){
        assert(!_module_stack.empty() && (_module_stack.top().md == ptr));
        assert(_module_stack.top().state == MODULE_INIT);
        ptr->setStage(MODEL_GLOB_INITED);
        _module_stack.top().state = MODULE_END_GLOB_DECLARE;
        _module_stack.pop();

        logMF(ptr, "module is finalize init component");
    }



    void ModelController::on_module_init_designFlow(Module* ptr) {
        assert(ptr->getStage() == MODEL_GLOB_INITED);
        assert(get_target_module_ele().state == MODULE_INIT_DESIGN_FLOW);
        assert(ptr != nullptr);
        /** push to model stack */
        _module_stack.push(Module_Stack_Element{ptr, MODULE_INIT_DESIGN_FLOW});
        /** debug value*/
        logMF(ptr,
              "module is initializing design flow");
        /** flow the program*/
        ptr->buildAll();

    }

    void ModelController::on_module_final(Module* ptr) {
        assert(!_module_stack.empty() && ( _module_stack.top().md == ptr ));
        assert(_module_stack.top().state == MODULE_INIT_DESIGN_FLOW);
        ptr->setStage(MODEL_FLOW_INITED);
        _module_stack.top().state = MODULE_END;
        assert(!is_top_fb_belong_to_top_module());
        _module_stack.pop();
        logMF(ptr,
              "module is finalized design flow");
    }



}