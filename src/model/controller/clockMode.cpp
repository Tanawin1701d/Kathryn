//
// Created by tanawin on 19/9/25.
//

#include "clockMode.h"

namespace kathryn{

    CLOCK_MODE cur_clock_mode = CM_POSEDGE;

    void SET_CLK_MODE(CLOCK_MODE cm){
        cur_clock_mode = cm;
    }

    void SET_CLK_MODE2NEG_EDGE(){
        cur_clock_mode = CM_NEGEDGE;
    }

    void SET_CLK_MODE2DEF(){
        cur_clock_mode = CM_POSEDGE;
    }

    CLOCK_MODE GET_CLOCK_MODE(){
        return cur_clock_mode;
    }


}