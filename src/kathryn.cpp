//
// Created by tanawin on 7/2/2567.
//


#include "kathryn.h"

namespace kathryn{



    void startModelKathryn(){
        getControllerPtr()->start();
    }

    void resetKathryn(){

        /** flush model formation log value first*/
        // flushMfDebugger();

        /** reset model layer*/
        getControllerPtr()->reset();
        /** reset simulation layer*/
        getSimController()->reset();
        /** clean global io declaration*/
        cleanGlobPool();


    }

    PARAM readParamKathryn(std::string filePath){
        ParamReader paramReader(std::move(filePath));
        PARAM params = paramReader.getKeyVal();

        return params;
    }



}
