//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ALU_H
#define KATHRYN_SRC_EXAMPLE_O3_ALU_H

#include "isaParam.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    inline opr& alu(opr& op, opr& srcA, opr& srcB){
        mWire(out, DATA_LEN);
        mWire(shamt, SHAMT_WIDTH);
        mVal(fullBit, 32, (ull(-1)));

        zif(op == ALU_OP_ADD)  out = srcA + srcB;
        zif(op == ALU_OP_SLL)  out = srcA << shamt;
        zif(op == ALU_OP_XOR)  out = srcA ^ srcB;
        zif(op == ALU_OP_OR)   out = srcA | srcB;
        zif(op == ALU_OP_AND)  out = srcA & srcB;
        zif(op == ALU_OP_SRL)  out = srcA >> shamt;
        zif(op == ALU_OP_SEQ)  out = (srcA == srcB).uext(DATA_LEN);
        zif(op == ALU_OP_SNE)  out = (srcA != srcB).uext(DATA_LEN);
        zif(op == ALU_OP_SUB)  out = srcA - srcB;
        zif(op == ALU_OP_SRA)  out = (~(fullBit >> shamt) | (srcA >> shamt));
        zif(op == ALU_OP_SLT)  out =
            ((srcA.sl(31) & ~srcB.sl(31)) |
            ((srcA.sl(31) == srcB.sl(31)) &
            (srcA.sl(0, 31) < srcB.sl(0, 31)))).uext(DATA_LEN);
        zif(op == ALU_OP_SGE)  out =
            ( (~srcA.sl(31) & srcB.sl(31)) |
             ((srcA.sl(31) == srcB.sl(31)) &
            (srcA.sl(0, 31) >= srcB.sl(0, 31)))).uext(DATA_LEN);;
        zif(op == ALU_OP_SLTU) out = srcA < srcB;
        zif(op == ALU_OP_SGEU) out = srcA >= srcB;
    }

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ALU_H
