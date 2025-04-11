//
// Created by tanawin on 8/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_EXEC_ALU_H
#define KATHRYN_SRC_KRIDE_EXEC_ALU_H

#include "kride/incl.h"

namespace kathryn{




    struct ALU{
        mWire(res, DATA_LEN);
        Operable& exec(Operable& aluOp, Operable& opr1, Operable& opr2){

            auto& shamt = *opr2.doSlice({0, SHAMT_WIDTH});

            zif(aluOp == ALU_OP_ADD ){res = opr1 + opr2;}
            zif(aluOp == ALU_OP_SLL ){res = opr1 << shamt;}
            zif(aluOp == ALU_OP_XOR ){res = opr1 ^ opr2;}
            zif(aluOp == ALU_OP_OR  ){res = opr1 | opr2;}
            zif(aluOp == ALU_OP_AND ){res = opr1 & opr2;}
            zif(aluOp == ALU_OP_SRL ){res = opr1 >> shamt;}
            zif(aluOp == ALU_OP_SEQ ){res(0) = (opr1 == opr2); res(1, DATA_LEN) = 0;}
            zif(aluOp == ALU_OP_SNE ){res(0) = (opr1 != opr2); res(1, DATA_LEN) = 0;}
            zif(aluOp == ALU_OP_SUB ){res = opr1 - opr2;}
            zif(aluOp == ALU_OP_SRA ){res = (opr1 >> shamt) |
                (opr1.doSlice({DATA_LEN-1, DATA_LEN})->extB(DATA_LEN) << shamt);} //// TODO pump sign bit
            zif(aluOp == ALU_OP_SLT ){res = opr1.slt(opr2); res(1, DATA_LEN) = 0;}
            zif(aluOp == ALU_OP_SGE ){res = opr1.sgt(opr2); res(1, DATA_LEN) = 0;}
            zif(aluOp == ALU_OP_SLTU){res = (opr1 < opr2); res(1, DATA_LEN) = 0;}
            zif(aluOp == ALU_OP_SGEU){res = (opr1 > opr2); res(1, DATA_LEN) = 0;}


        }
    };

    inline Operable& srcASel(Wire& selARes, Slot& slot){
        auto& sel = slot.at("srcASel");
        zif(sel == SRC_A_RS1)  { selARes = slot.at("src1");}
        zif(sel == SRC_A_PC)   { selARes = slot.at("pc");}
        zif(sel == SRC_A_ZERO) { selARes = 0; }

        return selARes;
    }

    inline Operable& srcBSel(Wire& selBRes, Slot& slot){
        auto& sel = slot.at("srcBSel");
        zif(sel == SRC_B_RS2)  { selBRes = slot.at("src2");}
        zif(sel == SRC_B_IMM)  { selBRes = slot.at("imm"); }
        zif(sel == SRC_B_FOUR) { selBRes = 4; }
        zif(sel == SRC_B_ZERO) { selBRes = 0; }
        return selBRes;
    }




}

#endif //KATHRYN_SRC_KRIDE_EXEC_ALU_H
