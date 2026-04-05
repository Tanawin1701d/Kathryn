//
// Created by tanawin on 7/8/2024.
//
#include "kathryn.h"
#include "riscvGen.h"
#include "example/riscv/core/core.h"

namespace kathryn{

    namespace riscv{

        void RISCV_GEN_MNG::startGen(PARAM& params){

            mMod(riscCore, Riscv, false);
            startModelKathryn();
            GenController* genCtrl = getGenControllerPtr();
            assert(genCtrl != nullptr);
            genCtrl->initEnv(params);
            genCtrl->start();
            //genCtrl->startSynthesis();
            resetKathryn();
        }

    }


}