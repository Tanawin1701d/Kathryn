//
// Created by tanawin on 19/9/25.
//

#include "clockMode.h"

namespace kathryn{

    CLOCK_MODE curClockMode = CM_POSEDGE;

    void SET_CLK_MODE(CLOCK_MODE cm){
        curClockMode = cm;
    }

    CLOCK_MODE GET_CLOCK_MODE(){
        return curClockMode;
    }


}