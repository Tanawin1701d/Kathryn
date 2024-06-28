//
// Created by tanawin on 26/6/2024.
//

#include "kathryn.h"
#include "genEle.h"

#include "test/autoGen/genMng.h"

namespace kathryn{


    GenEle::GenEle(int id): _id(id){
        addGenEle(this);
    }


    void GenEle::startGen(const PARAM& params){

        start(params);
        startModelKathryn();
        GenController* genCtrl = getGenController();
        assert(genCtrl != nullptr);
        genCtrl->initEnv(params);
        genCtrl->initEle();
        genCtrl->routeIo();
        genCtrl->generateEveryModule();
        genCtrl->reset();


    }

}
