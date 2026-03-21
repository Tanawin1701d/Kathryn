//
// Created by tanawin on 7/2/2567.
//


#include "kathryn.h"


namespace kathryn{



    void start_model_kathryn(){
        get_controller_ptr()->start();
    }

    void start_gen_kathryn(PARAM& params){
        GenController* gen_ctrl = get_gen_controller_ptr();
        assert(gen_ctrl != nullptr);
        gen_ctrl->init_env(params);
        gen_ctrl->start();
    }

    void reset_kathryn(){

        /** flush model formation log value first*/
        // flush_mf_debugger();

        /** clean global io declaration*/
        clean_glob_pool();
        /** reset model layer*/
        get_controller_ptr()->reset();
        /** reset simulation layer*/
        get_sim_controller_ptr()->reset();
        /** reset generator*/
        get_gen_controller_ptr()->reset();



    }

    PARAM read_param_kathryn(std::string file_path){
        ParamReader param_reader(std::move(file_path));
        PARAM params = param_reader.get_key_val();

        return params;
    }



}
