//
// Created by tanawin on 30/11/2566.
//

#include "controller.h"

namespace kathryn{

    /// central initializer
    ModelController*    _central_controller_ptr = nullptr;

    ModelController::ModelController() {
        /** to prevent loop allocation in global module constructor*/
        _central_controller_ptr = this;
        on_globalModule_init_component();
    }

    void ModelController::start(){
        assert(_global_module_ptr != nullptr);
        //// global Module must be auto initiated when controller is initialize or it is reset
        on_module_end_init_components  (_global_module_ptr);
        on_globalModule_init_designFlow();
        on_module_final                (_module_stack.top().md);
    }


    void ModelController::reset(){
        clean();
        on_globalModule_init_component();
    }

    void ModelController::clean(){
        /** delete old global module**/
        assert(is_all_flow_stack_empty());
        assert(_module_stack.empty());
        delete _global_module_ptr;
        _global_module_ptr = nullptr;
    }

    Module* ModelController::get_global_module_ptr(){
        assert(_global_module_ptr != nullptr);
        return _global_module_ptr;
    }

    ModelController* get_controller_ptr(){
        /// initiate controller before return
        /// lazy initializer
        if (_central_controller_ptr == nullptr){
            new ModelController();
            /// the constructor of model controller will handle itself
        }
        return _central_controller_ptr;
    }

    Module* get_global_module_ptr(){
        return get_controller_ptr()->get_global_module_ptr();
    }



    std::string ModelController::get_cur_model_stack_dbg() {

        std::vector<Module_Stack_Element> md_vec = cvtStackToVec(_module_stack);
        std::vector<FlowBlockBase*>       fb_vec = cvtStackToVec(_flow_block_stacks[FLOW_ST_BASE_STACK]);

        int accum_ident = 0;
        std::string result;

        /// retrive module hierachy
        for (auto mod: md_vec){
            result += (mod.md->getVarName() + "\n");
            result += genConString(' ', accum_ident);
            accum_ident += 4;
        }


        for (int i = 0; i < fb_vec.size(); i++){
            std::string position;
            if (i > 0){
                if (fb_vec[i]->get_flow_type() != CSELIF && fb_vec[i]->get_flow_type() != CSELSE) {
                    position += " subBlockIdx " + std::to_string(fb_vec[i - 1]->get_sub_blocks_ref().size()) + "    ";
                }
                position += " conBlockIdx " + std::to_string(fb_vec[i-1]->get_con_blocks_ref().size());
            }
            result += (fb_vec[i]->get_global_name() + "@ " + position + "\n" );
            result += genConString(' ', accum_ident);
            accum_ident += 4;
        }

        return result;

    }

}
