//
// Created by tanawin on 20/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IMMBRGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_IMMBRGEN_H


#include "isaParam.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    inline void immBrGen(Reg& instr, Wire& result){   ///DATA_HC DECODE

        mVal(zero1, 1, 0);        ///PARAM DECODE
        opr& opc = instr(0, 7);   ///DATA_CL DECODE

        opr& brOffset   = g(instr(31), instr(7), instr(25, 31),                ///DATA_CL DECODE
                            instr(8, 12), zero1).sext(ADDR_LEN);               ///DATA_CL DECODE
        opr& jalOffset  = g(instr(31), instr(12, 20), instr(20),               ///DATA_CL DECODE
                            instr(21, 31), zero1).sext(ADDR_LEN);              ///DATA_CL DECODE
        opr& jalrOffset = g(instr(31), instr(21, 31), zero1).sext(ADDR_LEN);   ///DATA_CL DECODE

        ztate(opc){                                    ///DATA_CL DECODE
            zcase(RV32_BRANCH){result = brOffset;}     ///DATA_DT DECODE
            zcase(RV32_JAL   ){result = jalOffset;}    ///DATA_DT DECODE
            zcase(RV32_JALR  ){result = jalrOffset;}   ///DATA_DT DECODE
            zcasedef{ result = 0; }                    ///DATA_DT DECODE
        }
    }

}

#endif //KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
