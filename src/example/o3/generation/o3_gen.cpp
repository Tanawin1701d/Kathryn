//
// Created by tanawin on 13/1/26.
//

#include "o3_gen.h"

#include "example/o3/core/core.h"

namespace kathryn::o3{

    void O3_GEN_MNG::start_gen(PARAM& params){

        m_mod(o3GenTop, Core, 0);
        start_model_kathryn();
        GenController* gen_ctrl = get_gen_controller_ptr();
        assert(gen_ctrl != nullptr);
        gen_ctrl->init_env(params);
        gen_ctrl->start();
        //gen_ctrl->start_synthesis();
        reset_kathryn();
    }



}