//
// Created by tanawin on 20/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IMMBRGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_IMMBRGEN_H


#include "isa_param.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    inline void imm_br_gen(Reg& instr, Wire& result){

        m_val(zero1, 1, 0);
        opr& opc = instr(0, 7);

        opr& br_offset   = g(instr(31), instr(7), instr(25, 31),
                            instr(8, 12), zero1).sext(ADDR_LEN);
        opr& jal_offset  = g(instr(31), instr(12, 20), instr(20),
                            instr(21, 31), zero1).sext(ADDR_LEN);
        opr& jalr_offset = g(instr(31), instr(21, 31), zero1).sext(ADDR_LEN);

        ztate(opc){
            zcase(RV32_BRANCH){result = br_offset;}
            zcase(RV32_JAL   ){result = jal_offset;}
            zcase(RV32_JALR  ){result = jalr_offset;}
            zcasedef{ result = 0; }
        }
    }

}

#endif //KATHRYN_SRC_EXAMPLE_O3_IMMGEN_H
