//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H


#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

        inline void immGen(Reg& instr, Reg& immType, Wire& result){


            mWire(lastExtended21, 21);
            mWire(lastExtended12, 12);

            zif(instr(31)){
                lastExtended21 = (1 << 21) - 1;
                lastExtended12 = (1 << 12) - 1;
            }zelse{
                lastExtended21 = 0;
                lastExtended12 = 0;
            }
            mVal(zero1, 1, 0);
            mVal(zero12, 12, 0);

            zif (immType == IMM_I)  {result = g( lastExtended21, instr(25, 31), instr(21, 25), instr(20));}
            zelif(immType == IMM_S) {result = g(lastExtended21, instr(25, 31), instr(8, 12), instr(7));}
            zelif(immType == IMM_U) {result = g(instr(31), instr(20, 31), instr(12, 20), zero12);}
            zelif(immType == IMM_J) {result = g(lastExtended12, instr(12, 20), instr(20), instr(25, 31), instr(21, 25), zero1);}
            zelse                   {result = g(lastExtended21, instr(25, 31), instr(21, 25), instr(20));}
        }



}

#endif //KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
