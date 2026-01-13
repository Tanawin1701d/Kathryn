//
// Created by tanawin on 13/1/26.
//

#include "O3_gen.h"

#include "example/o3/core/core.h"

namespace kathryn::o3{

    void O3_GEN_MNG::startGen(PARAM& params){

        mMod(o3GenTop, Core, 0);
        startModelKathryn();
        GenController* genCtrl = getGenController();
        assert(genCtrl != nullptr);
        genCtrl->initEnv(params);
        genCtrl->start();
        //genCtrl->startSynthesis();
        resetKathryn();
    }



}