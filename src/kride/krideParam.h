//
// Created by tanawin on 25/3/2025.
//

#ifndef SRC_KRIDE_kride_KRIDEPARAM_H
#define SRC_KRIDE_kride_KRIDEPARAM_H

namespace kathryn{

    constexpr int DATA_LEN   =    32;
    constexpr int INSTR_LEN   =    32;
    constexpr int ADDR_LEN   =    32;
    constexpr int ISSUE_NUM  =     2;

    constexpr int AMT_AREG   =    32;
    constexpr int AMT_PREG   =    64;

    constexpr int EXEC_WRITE_NUM = 5;
    constexpr int RENAME_NUM     = 2;
    constexpr int SPEC_TAG_LEN   = 5;
    constexpr int SPEC_TAG_SEL   = 3;

    constexpr int RRF_SEL = 6;
    constexpr int ARF_SEL = 5;


    /////// decoder section

    constexpr int DEC_AMT        = 2;
    /////// imm
    constexpr int IMM_TYPE_WIDTH = 2;
    constexpr int IMM_I          = 0;
    constexpr int IMM_S          = 1;
    constexpr int IMM_U          = 2;
    constexpr int IMM_J          = 3;
    ////// src a
    constexpr int SRC_A_SEL_WIDTH = 2;
    constexpr int SRC_A_RS1       = 0;
    constexpr int SRC_A_PC        = 1;
    constexpr int SRC_A_ZERO      = 2;
    ////// alu
    constexpr int ALU_OP_WIDTH    = 4;
    constexpr int RS_ENT_SEL      = 3;
    constexpr int ALU_OP_ADD      = 0;
    constexpr int ALU_OP_SLL      = 1;
    constexpr int ALU_OP_XOR      = 4;
    constexpr int ALU_OP_OR       = 6;
    constexpr int ALU_OP_AND      = 7;
    constexpr int ALU_OP_SRL      = 5;
    constexpr int ALU_OP_SEQ      = 8;
    constexpr int ALU_OP_SNE      = 9;
    constexpr int ALU_OP_SUB      = 10;
    constexpr int ALU_OP_SRA      = 11;
    constexpr int ALU_OP_SLT      = 12;
    constexpr int ALU_OP_SGE      = 13;
    constexpr int ALU_OP_SLTU     = 14;
    constexpr int ALU_OP_SGEU     = 15;


    constexpr int RV32_LOAD       = 0b0000011;
    constexpr int RV32_STORE      = 0b0100011;
    constexpr int RV32_MADD       = 0b1000011;
    constexpr int RV32_BRANCH     = 0b1100011;

    constexpr int RV32_LOAD_FP    = 0b0000111;
    constexpr int RV32_STORE_FP   = 0b0100111;
    constexpr int RV32_MSUB       = 0b1000111;
    constexpr int RV32_JALR       = 0b1100111;

    constexpr int RV32_CUSTOM_0   = 0b0001011;
    constexpr int RV32_CUSTOM_1   = 0b0101011;
    constexpr int RV32_NMSUB      = 0b1001011;
// 7'b1101011 is reserved

    constexpr int RV32_MISC_MEM   = 0b0001111;
    constexpr int RV32_AMO        = 0b0101111;
    constexpr int RV32_NMADD      = 0b1001111;
    constexpr int RV32_JAL        = 0b1101111;

    constexpr int RV32_OP_IMM     = 0b0010011;
    constexpr int RV32_OP         = 0b0110011;
    constexpr int RV32_OP_FP      = 0b1010011;
    constexpr int RV32_SYSTEM     = 0b1110011;

    constexpr int RV32_AUIPC      = 0b0010111;
    constexpr int RV32_LUI        = 0b0110111;
// 7'b1010111 is reserved
// 7'b1110111 is reserved

// 7'b0011011 is RV64-specific
// 7'b0111011 is RV64-specific
    constexpr int RV32_CUSTOM_2   = 0b1011011;
    constexpr int RV32_CUSTOM_3   = 0b1111011;

// Arithmetic FUNCT3 encodings

    constexpr int RV32_FUNCT3_ADD_SUB = 0;
    constexpr int RV32_FUNCT3_SLL     = 1;
    constexpr int RV32_FUNCT3_SLT     = 2;
    constexpr int RV32_FUNCT3_SLTU    = 3;
    constexpr int RV32_FUNCT3_XOR     = 4;
    constexpr int RV32_FUNCT3_SRA_SRL = 5;
    constexpr int RV32_FUNCT3_OR      = 6;
    constexpr int RV32_FUNCT3_AND     = 7;

// Branch FUNCT3 encodings

    constexpr int RV32_FUNCT3_BEQ   = 0;
    constexpr int RV32_FUNCT3_BNE   = 1;
    constexpr int RV32_FUNCT3_BLT   = 4;
    constexpr int RV32_FUNCT3_BGE   = 5;
    constexpr int RV32_FUNCT3_BLTU  = 6;
    constexpr int RV32_FUNCT3_BGEU  = 7;

// MISC-MEM FUNCT3 encodings
    `define RV32_FUNCT3_FENCE   0
    `define RV32_FUNCT3_FENCE_I 1

// SYSTEM FUNCT3 encodings

    `define RV32_FUNCT3_PRIV   0
    `define RV32_FUNCT3_CSRRW  1
    `define RV32_FUNCT3_CSRRS  2
    `define RV32_FUNCT3_CSRRC  3
    `define RV32_FUNCT3_CSRRWI 5
    `define RV32_FUNCT3_CSRRSI 6
    `define RV32_FUNCT3_CSRRCI 7

// PRIV FUNCT12 encodings

    `define RV32_FUNCT12_ECALL  12'b000000000000
    `define RV32_FUNCT12_EBREAK 12'b000000000001
    `define RV32_FUNCT12_ERET   12'b000100000000

// RV32M encodings
    `define RV32_FUNCT7_MUL_DIV 7'd1

    `define RV32_FUNCT3_MUL    3'd0
    `define RV32_FUNCT3_MULH   3'd1
    `define RV32_FUNCT3_MULHSU 3'd2
    `define RV32_FUNCT3_MULHU  3'd3
    `define RV32_FUNCT3_DIV    3'd4
    `define RV32_FUNCT3_DIVU   3'd5
    `define RV32_FUNCT3_REM    3'd6
    `define RV32_FUNCT3_REMU   3'd7

    ////// dmem
    constexpr int MEM_TYPE_WIDTH  = 3;
    constexpr int MEM_TYPE_LB     = 0;
    constexpr int MEM_TYPE_LH     = 1;
    constexpr int MEM_TYPE_LW     = 2;
    constexpr int MEM_TYPE_LD     = 3;
    constexpr int MEM_TYPE_LBU    = 4;
    constexpr int MEM_TYPE_LHU    = 5;
    constexpr int MEM_TYPE_LWU    = 6;

    constexpr int MEM_TYPE_SB     = 0;
    constexpr int MEM_TYPE_SH     = 1;
    constexpr int MEM_TYPE_SW     = 2;
    constexpr int MEM_TYPE_SD     = 3;

    constexpr int  MD_OP_WIDTH    = 2;
    constexpr int  MD_OP_MUL      = 0;
    constexpr int  MD_OP_DIV      = 1;
    constexpr int  MD_OP_REM      = 2;

    constexpr int MD_OUT_SEL_WIDTH  = 2;
    constexpr int MD_OUT_LO         = 0;
    constexpr int MD_OUT_HI         = 1;
    constexpr int MD_OUT_REM        = 2;




    //////////////////////////////////////
    //////////////////////////////////////
    //////////////////////////////////////


    constexpr char DEC[]   = "DEC";
    constexpr char EXEC[]  = "EXEC";
    constexpr char DP[]    = "DISPATCH";
    constexpr char ALLOC[] = "ALLOC"; /// TODO we won't use this

    constexpr char SPEC_GEN[]  = "SPECGEN";
    constexpr char RT[]        = "RT";
    constexpr char RF_FINDER[] = "RF_FINDER";



    constexpr char COMMIT[] = "COMMIT";

    constexpr char EXEC_BRANCH[] = "EXEC_BRANCH";

    constexpr char RRF_exec[]  = "rrf_exec";
    constexpr char RRF_alloc[] = "rrf_alloc";

    constexpr char MPFT_exec[] = "mpft_exb";

    constexpr char C_VALID[]     = "valid";
    constexpr char C_SPEC_PREF[] = "spec";

}

#endif //SRC_KRIDE_kride_KRIDEPARAM_H
