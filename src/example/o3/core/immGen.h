//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H


#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

        inline void immGen(Reg& instr, Reg& immType, Wire& result){

            mVal(zero1, 1, 0);
            mVal(zero12, 12, 0);

            zif (immType == IMM_I)  {result = g(instr(25, 31), instr(21, 25), instr(20)).sext(DATA_LEN);}
            zelif(immType == IMM_S) {result = g(instr(25, 31), instr(8, 12) , instr(7) ).sext(DATA_LEN);}
            zelif(immType == IMM_U) {result = g(    instr(31), instr(20, 31), instr(12, 20), zero12);}
            zelif(immType == IMM_J) {result = g(    instr(31), instr(12, 20), instr(20)    , instr(25, 31), instr(21, 25), zero1).sext(DATA_LEN);}
            zelse                   {result = g(instr(25, 31), instr(21, 25), instr(20)).sext(DATA_LEN);}
        }



}

#endif //KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
