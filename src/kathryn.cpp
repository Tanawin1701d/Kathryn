//
// Created by tanawin on 7/2/2567.
//


#include "kathryn.h"


namespace kathryn{

    void resetKathryn(){

        /** flush model formation log value first*/
        flushMfDebugger();
        /** reset model layer*/
        getControllerPtr()->reset();
        /** reset simulation layer*/
        getSimController()->reset();

    }

}