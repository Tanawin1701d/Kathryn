//
// Created by tanawin on 3/12/2566.
//

#include "model/hw_component/global_component/global_component.h"

#include "controller.h"

#include "util/logger/logger.h"

namespace kathryn{


    Module* ModelController::get_top_module_ptr() {
        /** base line must be auto initialized */
        assert(!module_stack.empty());
        return module_stack.top().md;
    }

    Module_Stack_Element& ModelController::get_target_module_ele() {
        assert(!module_stack.empty());
        return module_stack.top();
    }

    /**
     * state register handling
     *
     * */
    void ModelController::on_sp_reg_init(CtrlFlowRegBase* ptr, SP_REG_TYPE sp_reg_type) {
        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /**localize necessary destination*/
        target_module_ptr->add_sp_reg(ptr, sp_reg_type);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name(); //// build inherit name for that module
        if (ptr->require_reset_event())
            ptr->make_reset_event();
        /** debug value*/
        log_mf(ptr,
              "[" + sp_reg_type_to_str(sp_reg_type) + "] is initialized and set parent to "
              + target_module_ptr->get_ident_debug_value()
        );

    }

    /** register handling*/
    void ModelController::on_reg_init(Reg* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        /** assign reg to module */
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->add_user_reg(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();
        ////ptr->make_reset_event();
        /** debug value*/
        log_mf(ptr,
              "USER_REG is initialized and set parent to " + target_module_ptr->get_ident_debug_value());
    }

    void ModelController::on_reg_update(AsmNode* asm_node, Reg* src_reg){
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
            fb->add_element_in_flow_block(asm_node);
            log_mf(src_reg,
                  "user Reg is updating value @ fb block " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asm_node);
            // asm_node->dry_assign();
            // log_mf(src_reg,
            //       "user reg is updatting without flowblock");
            // Module* target_module_ptr = get_top_module_ptr();
            // target_module_ptr->add_node(asm_node);
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
        target_module_ptr->add_user_wires(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();
        ptr->make_def_event();

        /** debug value*/
        log_mf(ptr,
              "user wire is initialized and set parent to " + target_module_ptr->get_ident_debug_value());
    }

    void ModelController::on_wire_update(AsmNode* asm_node, Wire* src_wire) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asm_node != nullptr);
        try_purify_flow_stack();
        //assert(!flow_block_stack.empty());
        if (is_top_fb_belong_to_top_module()) {
            /**in flow block*/
            auto fb = get_top_flow_block_base_ptr();
            fb->add_element_in_flow_block(asm_node);
            log_mf(src_wire,
                  "user wire is updating @ fb " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asm_node);
            // asm_node->dry_assign();
            // log_mf(src_wire,
            //       "user wire is updatting without flowblock");
            // Module* target_module_ptr = get_top_module_ptr();
            // target_module_ptr->add_node(asm_node);
        }
    }

    /** expr_metas*/
    void ModelController::on_expression_init(expression* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->add_user_expression(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();

        /** debug value*/
        log_mf(ptr,
              "expr is initializing and set parent to " + target_module_ptr->get_ident_debug_value());
    }

    /** mem_block*/
    void ModelController::on_memBlk_init(MemBlock* ptr){
        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize it*/
        /** localize necessary destination*/
        target_module_ptr->add_user_mem_blk(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();
        /** debug value*/
        log_mf(ptr,
              "mem_blk is initializing and set parent to " + target_module_ptr->get_ident_debug_value());
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
            fb->add_element_in_flow_block(asm_node);
            log_mf(src_holder,
                  "mem_blk HOLDER is updating value @ fb block " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asm_node);
            // asm_node->dry_assign();
            // log_mf(src_holder,
            //       "user mem holder is updatting without flowblock");
            // Module* target_module_ptr = get_top_module_ptr();
            // target_module_ptr->add_node(asm_node);
        }

    }

    /** nest*/
    void ModelController::on_nest_init(nest* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->add_user_nest(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();

        /** debug value*/
        log_mf(ptr,
              "nest is initializing and set parent to " + target_module_ptr->get_ident_debug_value());
    }

    void ModelController::on_nest_update(AsmNode* asm_node, nest* src_nest) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asm_node != nullptr);
        try_purify_flow_stack();
        //assert(!flow_block_stack.empty());
        if (is_top_fb_belong_to_top_module()) {
            /**in flow block*/
            auto fb = get_top_flow_block_base_ptr();
            fb->add_element_in_flow_block(asm_node);
            log_mf(src_nest,
                  "user nest is updating @ fb " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asm_node);
            // asm_node->dry_assign();
            // log_mf(src_nest,
            //       "user nest is updating without flowblock");
            // Module* target_module_ptr = get_top_module_ptr();
            // target_module_ptr->add_node(asm_node);
        }
    }


    /** value*/
    void ModelController::on_value_init(Val* ptr) {
        assert(ptr != nullptr);
        on_box_tryAddToBox(ptr, ptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->add_user_val(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();
        /** debug value*/
        log_mf(ptr,
              "val is initializing and set parent to " + target_module_ptr->get_ident_debug_value());
    }

    /** pm_value*/
    void ModelController::on_pmValue_init(PmVal* ptr) {
        assert(ptr != nullptr);
        assert(box_stack.empty());
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->add_user_pm_val(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();
        log_mf(ptr, "pmval is initializing and set parent to " + target_module_ptr->get_ident_debug_value());

    }

    /** box*/
    void ModelController::on_box_init(Box* ptr) {
        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        ////// if it is slave block do not push it to module storage
        ptr->set_parent(target_module_ptr);
        if (box_stack.empty()){
            target_module_ptr->add_user_box(ptr);
        }else{
            Box* top_box = box_stack.top();
            top_box->add_sub_box(ptr);
        }
        ptr->build_inherit_name();

        box_stack.push(ptr);
        /** debug value*/
        log_mf(ptr, "box is initializing and set parent to " +
                   target_module_ptr->get_ident_debug_value());
    }

    void ModelController::on_box_end_init(Box* ptr){
        /**check that the stack is not do somthing wrong*/
        assert(ptr == box_stack.top());
        box_stack.pop();
        log_mf(ptr, "box is pick out from stack");
    }

    void ModelController::on_box_update(AsmNode* asm_node, Box* src_box) {
        /**
         * please note that UpdateEvent should fill update value/ and slice
         * but it must let update condition and state as nullptr to let block fill
         * to it
         * */
        /*** do not add to module any more*/
        assert(asm_node != nullptr);
        try_purify_flow_stack();
        //assert(!flow_block_stack.empty());
        if (is_top_fb_belong_to_top_module()) {
            /**in flow block*/
            auto fb = get_top_flow_block_base_ptr();
            fb->add_element_in_flow_block(asm_node);
            log_mf(src_box,
                  "user nest is updating @ fb " + fb->get_md_ident_val());
        }else{
            ///// no worry about memory leak here because flow stack acknowledge it
            new FlowBlockPseudo(asm_node);
            // asm_node->dry_assign();
            // log_mf(src_box,
            //       "user nest is updating without flowblock");
            // Module* target_module_ptr = get_top_module_ptr();
            // target_module_ptr->add_node(asm_node);
        }
    }

    void ModelController::on_box_tryAddToBox(Operable* opr1, Assignable* asb){
        assert(asb  != nullptr);
        assert(opr1 != nullptr);
        /**check that box is being fomation and make sure reg is from user */
        if ( (!box_stack.empty()) && opr1->cast_to_ident()->is_user_var()){
            box_stack.top()->add_nest_meta({opr1, asb});
        }
    }


    void ModelController::on_itf_init(ModelInterface* ptr){

        assert(ptr != nullptr);
        Module* target_module_ptr = get_top_module_ptr();
        /** localize necessary destination*/
        target_module_ptr->add_user_itf(ptr);
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();
        /** debug value*/
        log_mf(ptr,
              "val is initializing and set parent to " + target_module_ptr->get_ident_debug_value());

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
        global_module_ptr = new Module(false);
        global_module_ptr->set_var_name("globe_mod");
        /** for global module for initialize project*/
        assert(global_module_ptr != nullptr);
        module_stack.push(Module_Stack_Element{global_module_ptr, MODULE_INIT});
        global_module_ptr->build_inherit_name();
        initiate_global_component();
    }

    void ModelController::on_globalModule_init_designFlow(){
        assert(global_module_ptr != nullptr);
        assert(module_stack.empty());
        assert(is_all_flow_stack_empty());
        module_stack.push(Module_Stack_Element{global_module_ptr, MODULE_INIT_DESIGN_FLOW});
        global_module_ptr->build_all();
    }

    void ModelController::on_globalModule_init_auxilaryComponent(){

        assert(global_module_ptr != nullptr);
        assert(module_stack.empty());
        assert(is_all_flow_stack_empty());
        module_stack.push(Module_Stack_Element{global_module_ptr, MODULE_INIT_AUX});

    }

    void ModelController::on_globalModule_final_auxilaryComponent(){

        assert(!module_stack.empty());
        assert(module_stack.top().state == MODULE_INIT_AUX);
        module_stack.top().state = MODULE_INIT_AUX;
        module_stack.pop();

    }








    ///////// normal module


    void ModelController::on_module_init_components(Module* ptr) {
        /**check that module initialization is in construct state not in designflow constructing*/
        assert(get_target_module_ele().state == MODULE_INIT || get_target_module_ele().state == MODULE_INIT_DESIGN_FLOW);
        assert(ptr->get_stage() == MODEL_UNINIT);
        /** previous module*/
        Module* target_module_ptr = get_top_module_ptr();
        target_module_ptr->add_user_sub_module(ptr);
        /** current module*/
        /**at least module must be other submodule*/
        module_stack.push(Module_Stack_Element{ptr, MODULE_INIT});
        ptr->set_parent(target_module_ptr);
        ptr->build_inherit_name();
        /** debug value*/
        log_mf(ptr,
              "module is initializing and set parent to " + target_module_ptr->get_ident_debug_value());
    }

    void ModelController::on_module_end_init_components(Module* ptr){
        assert(!module_stack.empty() && (module_stack.top().md == ptr));
        assert(module_stack.top().state == MODULE_INIT);
        ptr->set_stage(MODEL_GLOB_INITED);
        module_stack.top().state = MODULE_END_GLOB_DECLARE;
        module_stack.pop();

        log_mf(ptr, "module is finalize init component");
    }



    void ModelController::on_module_init_designFlow(Module* ptr) {
        assert(ptr->get_stage() == MODEL_GLOB_INITED);
        assert(get_target_module_ele().state == MODULE_INIT_DESIGN_FLOW);
        assert(ptr != nullptr);
        /** push to model stack */
        module_stack.push(Module_Stack_Element{ptr, MODULE_INIT_DESIGN_FLOW});
        /** debug value*/
        log_mf(ptr,
              "module is initializing design flow");
        /** flow the program*/
        ptr->build_all();

    }

    void ModelController::on_module_final(Module* ptr) {
        assert(!module_stack.empty() && ( module_stack.top().md == ptr ));
        assert(module_stack.top().state == MODULE_INIT_DESIGN_FLOW);
        ptr->set_stage(MODEL_FLOW_INITED);
        module_stack.top().state = MODULE_END;
        assert(!is_top_fb_belong_to_top_module());
        module_stack.pop();
        log_mf(ptr,
              "module is finalized design flow");
    }



}