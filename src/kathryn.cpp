//
// Created by tanawin on 7/2/2567.
//


#include "kathryn.h"

namespace kathryn{



    void startModelKathryn(){
        getControllerPtr()->start();
    }

    void startGenKathryn(PARAM& params){
        GenController* genCtrl = getGenController();
        assert(genCtrl != nullptr);
        genCtrl->initEnv(params);
        genCtrl->start();
    }

    void resetKathryn(){

        /** flush model formation log value first*/
        // flushMfDebugger();

        /** clean global io declaration*/
        cleanGlobPool();
        /** reset model layer*/
        getControllerPtr()->reset();
        /** reset simulation layer*/
        getSimController()->reset();
        /** reset generator*/
        getGenController()->reset();


    }

    PARAM readParamKathryn(std::string filePath){
        ParamReader paramReader(std::move(filePath));
        PARAM params = paramReader.getKeyVal();

        return params;
    }



}
