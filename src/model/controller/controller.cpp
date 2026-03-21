//
// Created by tanawin on 30/11/2566.
//

#include "controller.h"

namespace kathryn{

    /** central initializer*/
    ModelController*    central_controller_ptr = nullptr;
    ///Module*             global_module_ptr      = nullptr;


    ModelController::ModelController() {
        /** to prevent loop allocation in global module constructor*/
        central_controller_ptr = this;
        on_globalModule_init_component();
    }

    void ModelController::start(){
        assert(global_module_ptr != nullptr);
        /***
         * global Module must be auto initiated when controller is initialize or it is reset
         * */
        on_module_end_init_components(global_module_ptr);
        on_globalModule_init_designFlow();
        on_module_final(module_stack.top().md);
    }


    void ModelController::reset(){
        clean();
        on_globalModule_init_component();
    }

    void ModelController::clean(){
        /** delete old global module**/
        assert(is_all_flow_stack_empty());
        assert(module_stack.empty());
        delete global_module_ptr;
        global_module_ptr = nullptr;
    }

    Module* ModelController::get_global_module_ptr(){
        assert(global_module_ptr != nullptr);
        return global_module_ptr;
    }

    ModelController* get_controller_ptr(){
        /// initiate controller before return
        /***lazy initializer*/
        if (central_controller_ptr == nullptr){
            new ModelController();
            /** the constructor of model controller will handle itself*/
        }
        return central_controller_ptr;
    }

    Module* get_global_module_ptr(){
        return get_controller_ptr()->get_global_module_ptr();
    }



    std::string ModelController::get_cur_model_stack() {

        std::vector<Module_Stack_Element> md_vec = cvt_stack_to_vec(module_stack);
        std::vector<FlowBlockBase*>       fb_vec = cvt_stack_to_vec(flow_block_stacks[FLOW_ST_BASE_STACK]);

        int accum_ident = 0;
        std::string result;

        for (auto mod: md_vec){
            result += (mod.md->get_var_name() + "\n");
            result += gen_con_string(' ', accum_ident);
            accum_ident += 4;
        }

        for (int i = 0; i < fb_vec.size(); i++){
            std::string position;
            if (i > 0){
                if (fb_vec[i]->get_flow_type() != CSELIF && fb_vec[i]->get_flow_type() != CSELSE) {
                    position += " sub_block_idx " + std::to_string(fb_vec[i - 1]->get_sub_blocks().size()) + "    ";
                }
                position += " con_block_idx " + std::to_string(fb_vec[i-1]->get_con_blocks().size());
            }
            result += (fb_vec[i]->get_global_name() + "@ " + position + "\n" );
            result += gen_con_string(' ', accum_ident);
            accum_ident += 4;
        }

        return result;

    }



//    void free_controller_ptr(){
//        ///// finalize global module if it have
//        if (central_controller_ptr != nullptr){
//            central_controller_ptr->on_module_final(global_module_ptr);
//        }
//    }



}
