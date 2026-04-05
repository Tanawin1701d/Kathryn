//
// Created by tanawin on 19/9/25.
//

#include "clockMode.h"

namespace kathryn{

    CLOCK_MODE curClockMode = CM_POSEDGE;

    void SET_CLK_MODE(CLOCK_MODE cm){
        curClockMode = cm;
    }

    void SET_CLK_MODE2NEG_EDGE(){
        curClockMode = CM_NEGEDGE;
    }

    void SET_CLK_MODE2DEF(){
        curClockMode = CM_POSEDGE;
    }

    CLOCK_MODE GET_CLOCK_MODE(){
        return curClockMode;
    }


}