//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ALU_H
#define KATHRYN_SRC_EXAMPLE_O3_ALU_H

#include "isa_param.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    inline opr& alu(opr& op, opr& src_a, opr& src_b){
        m_wire(out, DATA_LEN);
        m_wire(shamt, SHAMT_WIDTH);

        shamt = src_b.sl(0, SHAMT_WIDTH);
        /////// for shift right arithmatic
        opr& a_msb = src_a.sl(DATA_LEN-1);
        opr& a_msb_ext = a_msb.ext_b(DATA_LEN);
        m_val(full_bit, DATA_LEN, (ull)(-1));
        opr& sign_mask = ~(full_bit >> shamt);

        ztate(op){
            zcase(ALU_OP_ADD)  { out = src_a + src_b;                                      }
            zcase(ALU_OP_SLL)  { out = src_a << shamt;                                    }
            zcase(ALU_OP_XOR)  { out = src_a ^ src_b;                                      }
            zcase(ALU_OP_OR)   { out = src_a | src_b;                                      }
            zcase(ALU_OP_AND)  { out = src_a & src_b;                                      }
            zcase(ALU_OP_SRL)  { out = src_a >> shamt;                                    }
            zcase(ALU_OP_SEQ)  { out = (src_a == src_b).uext(DATA_LEN);                    }
            zcase(ALU_OP_SNE)  { out = (src_a != src_b).uext(DATA_LEN);                    }
            zcase(ALU_OP_SUB)  { out = src_a - src_b;                                      }
            zcase(ALU_OP_SRA)  { out = ((sign_mask & a_msb_ext) | (src_a >> shamt));         }
            zcase(ALU_OP_SLT)  { out = (src_a.slt(src_b)).uext(DATA_LEN);                  }
            zcase(ALU_OP_SGE)  { out = (src_a.sgt(src_b) | (src_a == src_b)).uext(DATA_LEN); }
            zcase(ALU_OP_SLTU) { out = src_a < src_b;                                      }
            zcase(ALU_OP_SGEU) { out = src_a >= src_b;                                     }
        }
        return out;
    }

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ALU_H
