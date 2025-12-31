//
// Created by tanawin on 20/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IMMBRGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_IMMBRGEN_H


#include "isaParam.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    inline void immBrGen(Reg& instr, Wire& result){

        mVal(zero1, 1, 0);
        opr& opc = instr(0, 7);

        opr& brOffset  = g(instr(31), instr(7), instr(25, 31),
            instr(8, 12), zero1).sext(ADDR_LEN);
        opr& jalOffset = g(instr(31), instr(12, 20), instr(20),
            instr(21, 31), zero1).sext(ADDR_LEN);
        opr& jalrOffset = g(instr(31), instr(21, 31), zero1).sext(ADDR_LEN);

        zif  (opc == RV32_BRANCH){result = brOffset;}
        zelif(opc == RV32_JAL   ){result = jalOffset;}
        zelif(opc == RV32_JALR  ){result = jalrOffset;}
        zelse{ result = 0; }
    }

}

#endif //KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
