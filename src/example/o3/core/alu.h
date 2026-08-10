//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ALU_H
#define KATHRYN_SRC_EXAMPLE_O3_ALU_H

#include "isaParam.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    inline opr& alu(opr& op, opr& srcA, opr& srcB){   ///DATA_HC SHARED_COMP
        mWire(out, DATA_LEN);   ///DATA_HWD SHARED_COMP
        mWire(shamt, SHAMT_WIDTH);   ///DATA_HWD SHARED_COMP

        shamt = srcB.sl(0, SHAMT_WIDTH);   ///DATA_CL SHARED_COMP
        /////// for shift right arithmatic
        opr& aMsb = srcA.sl(DATA_LEN-1);   ///DATA_CL SHARED_COMP
        opr& aMsbExt = aMsb.extB(DATA_LEN);   ///DATA_CL SHARED_COMP
        mVal(fullBit, DATA_LEN, (ull)(-1));   ///PARAM SHARED_COMP
        opr& signMask = ~(fullBit >> shamt);   ///DATA_CL SHARED_COMP

        ztate(op){   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_ADD)  { out = srcA + srcB;                                      }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SLL)  { out = srcA << shamt;                                    }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_XOR)  { out = srcA ^ srcB;                                      }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_OR)   { out = srcA | srcB;                                      }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_AND)  { out = srcA & srcB;                                      }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SRL)  { out = srcA >> shamt;                                    }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SEQ)  { out = (srcA == srcB).uext(DATA_LEN);                    }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SNE)  { out = (srcA != srcB).uext(DATA_LEN);                    }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SUB)  { out = srcA - srcB;                                      }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SRA)  { out = ((signMask & aMsbExt) | (srcA >> shamt));         }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SLT)  { out = (srcA.slt(srcB)).uext(DATA_LEN);                  }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SGE)  { out = (srcA.sgt(srcB) | (srcA == srcB)).uext(DATA_LEN); }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SLTU) { out = srcA < srcB;                                      }   ///DATA_CL SHARED_COMP
            zcase(ALU_OP_SGEU) { out = srcA >= srcB;                                     }   ///DATA_CL SHARED_COMP
        }
        return out;   ///DATA_HC SHARED_COMP
    }

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ALU_H
