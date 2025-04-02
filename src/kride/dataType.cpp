//
// Created by tanawin on 2/4/2025.
//

#include "dataType.h"

namespace kathryn{

    void D_DECODE::decode(Reg &instr) {

        auto& op     = instr(0, 7);
        auto& funct7 = instr(31,25);
        auto& funct12= instr(31,20);
        auto& funct3 = instr(14,12);

        auto intDec = dec(intDecMeta);


        intDec <<= {0    , ALU_OP_ADD, RS_ENT_ALU,
                    IMM_I, SRC_A_RS1 , SRC_B_IMM,
                    0    , 1         , 0};


        zif(op == RV32_LOAD)
            intDec({"rsEntry", "regWr"}) <<= {RS_ENT_LDST, 1};
        zif(op == RV32_STORE)
            intDec({"rsEntry", "immType", "useRs2"}) <<= {RS_ENT_LDST, IMM_S, 1};
        zif(op == RV32_BRANCH){
            intDec({"rsEntry", "srcBSel", "useRs2"}) <<= {RS_ENT_BRANCH, SRC_B_RS2, 1};
            zif(funct3 == RV32_FUNCT3_BEQ  ) intDec({"aluOp"}) <<= {ALU_OP_SEQ};
            zif(funct3 == RV32_FUNCT3_BNE  ) intDec({"aluOp"}) <<= {ALU_OP_SNE};
            zif(funct3 == RV32_FUNCT3_BLT  ) intDec({"aluOp"}) <<= {ALU_OP_SLT};
            zif(funct3 == RV32_FUNCT3_BLTU ) intDec({"aluOp"}) <<= {ALU_OP_SLTU};
            zif(funct3 == RV32_FUNCT3_BGE  ) intDec({"aluOp"}) <<= {ALU_OP_SGE};
            zif(funct3 == RV32_FUNCT3_BGEU ) intDec({"aluOp"}) <<= {ALU_OP_SGEU};
            intDec({"illInstr"}) <<= {ALU_OP_SGEU};
        }
        zif(op == RV32_JAL)
            intDec({"rsEntry", "srcASel" , "srcBSel", "regWr", "useRs1"}) <<=
                {RS_ENT_JAL, SRC_A_PC, SRC_B_FOUR, 1, 0};

        zif(op == RV32_JALR)
            intDec({"rsEntry", "srcASel" , "srcBSel", "regWr", "useRs1"}) <<=
                {RS_ENT_JAL, SRC_A_PC, SRC_B_FOUR, 1, 0};



    }

}