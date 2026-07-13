//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H

#include "parameter.h"

namespace kathryn::o3{

        inline void immGen(Reg& instr, Reg& immType, Wire& result){   ///DATA_HC DECODE

            mVal(zero1, 1, 0);   ///PARAM DECODE
            mVal(zero12, 12, 0);   ///PARAM DECODE

            ztate(immType){   ///DATA_CL DECODE
                zcase(IMM_I) {result = g(instr(25, 32), instr(21, 25), instr(20)).sext(DATA_LEN);}   ///DATA_CL DECODE
                zcase(IMM_S) {result = g(instr(25, 32), instr(8, 12) , instr(7) ).sext(DATA_LEN);}   ///DATA_CL DECODE
                zcase(IMM_U) {result = g(    instr(31), instr(20, 31), instr(12, 20), zero12);}   ///DATA_CL DECODE
                zcase(IMM_J) {result = g(    instr(31), instr(12, 20), instr(20)    , instr(25, 31), instr(21, 25), zero1).sext(DATA_LEN);}   ///DATA_CL DECODE
                zcasedef     {result = g(instr(25, 32), instr(21, 25), instr(20)).sext(DATA_LEN);}   ///DATA_CL DECODE
            }
        }



}

#endif //KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
