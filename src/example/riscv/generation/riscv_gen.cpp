//
// Created by tanawin on 7/8/2024.
//
#include "kathryn.h"
#include "riscv_gen.h"
#include "example/riscv/core/core.h"

namespace kathryn{

    namespace riscv{

        void RISCV_GEN_MNG::start_gen(PARAM& params){

            m_mod(risc_core, Riscv, false);
            start_model_kathryn();
            GenController* gen_ctrl = get_gen_controller_ptr();
            assert(gen_ctrl != nullptr);
            gen_ctrl->init_env(params);
            gen_ctrl->start();
            //gen_ctrl->start_synthesis();
            reset_kathryn();
        }

    }


}