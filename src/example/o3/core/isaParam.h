//
// Created by tanawin on 26/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ISAPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_ISAPARAM_H

namespace kathryn::o3{
    // Basic parameters
    constexpr int INST_WIDTH      = 32;   ///PARAM DECODE
    constexpr int REG_ADDR_WIDTH  = 5;   ///PARAM DECODE
    constexpr int XPR_LEN         = 32;   ///PARAM DECODE
    constexpr int DOUBLE_XPR_LEN  = 64;   ///PARAM DECODE
    constexpr int LOG2_XPR_LEN    = 5;   ///PARAM DECODE
    constexpr int SHAMT_WIDTH     = 5;   ///PARAM DECODE

    constexpr int RV_NOP = 0b0010011;   ///PARAM DECODE

    // Opcodes
    constexpr int RV32_LOAD   = 0b0000011;   ///PARAM DECODE
    constexpr int RV32_STORE  = 0b0100011;   ///PARAM DECODE
    constexpr int RV32_MADD   = 0b1000011;   ///PARAM DECODE
    constexpr int RV32_BRANCH = 0b1100011;   ///PARAM DECODE

    constexpr int RV32_LOAD_FP  = 0b0000111;   ///PARAM DECODE
    constexpr int RV32_STORE_FP = 0b0100111;   ///PARAM DECODE
    constexpr int RV32_MSUB     = 0b1000111;   ///PARAM DECODE
    constexpr int RV32_JALR     = 0b1100111;   ///PARAM DECODE

    constexpr int RV32_CUSTOM_0 = 0b0001011;   ///PARAM DECODE
    constexpr int RV32_CUSTOM_1 = 0b0101011;   ///PARAM DECODE
    constexpr int RV32_NMSUB    = 0b1001011;   ///PARAM DECODE
    // 0b1101011 is reserved

    constexpr int RV32_MISC_MEM = 0b0001111;   ///PARAM DECODE
    constexpr int RV32_AMO      = 0b0101111;   ///PARAM DECODE
    constexpr int RV32_NMADD    = 0b1001111;   ///PARAM DECODE
    constexpr int RV32_JAL      = 0b1101111;   ///PARAM DECODE

    constexpr int RV32_OP_IMM   = 0b0010011;   ///PARAM DECODE
    constexpr int RV32_OP       = 0b0110011;   ///PARAM DECODE
    constexpr int RV32_OP_FP    = 0b1010011;   ///PARAM DECODE
    constexpr int RV32_SYSTEM   = 0b1110011;   ///PARAM DECODE

    constexpr int RV32_AUIPC    = 0b0010111;   ///PARAM DECODE
    constexpr int RV32_LUI      = 0b0110111;   ///PARAM DECODE
    // 0b1010111 is reserved
    // 0b1110111 is reserved

    // 0b0011011 is RV64-specific
    // 0b0111011 is RV64-specific
    constexpr int RV32_CUSTOM_2 = 0b1011011;   ///PARAM DECODE
    constexpr int RV32_CUSTOM_3 = 0b1111011;   ///PARAM DECODE

    // Arithmetic FUNCT3 encodings
    constexpr int RV32_FUNCT3_ADD_SUB  = 0;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_SLL      = 1;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_SLT      = 2;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_SLTU     = 3;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_XOR      = 4;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_SRA_SRL  = 5;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_OR       = 6;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_AND      = 7;   ///PARAM DECODE

    // Branch FUNCT3 encodings
    constexpr int RV32_FUNCT3_BEQ  = 0;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_BNE  = 1;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_BLT  = 4;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_BGE  = 5;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_BLTU = 6;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_BGEU = 7;   ///PARAM DECODE

    // MISC-MEM FUNCT3 encodings
    constexpr int RV32_FUNCT3_FENCE   = 0;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_FENCE_I = 1;   ///PARAM DECODE

    // SYSTEM FUNCT3 encodings
    constexpr int RV32_FUNCT3_PRIV   = 0;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_CSRRW  = 1;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_CSRRS  = 2;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_CSRRC  = 3;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_CSRRWI = 5;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_CSRRSI = 6;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_CSRRCI = 7;   ///PARAM DECODE

    // PRIV FUNCT12 encodings
    constexpr int RV32_FUNCT12_ECALL  = 0b000000000000;   ///PARAM DECODE
    constexpr int RV32_FUNCT12_EBREAK = 0b000000000001;   ///PARAM DECODE
    constexpr int RV32_FUNCT12_ERET   = 0b000100000000;   ///PARAM DECODE

    // RV32M encodings
    constexpr int RV32_FUNCT7_MUL_DIV = 1;   ///PARAM DECODE

    constexpr int RV32_FUNCT3_MUL    = 0;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_MULH   = 1;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_MULHSU = 2;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_MULHU  = 3;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_DIV    = 4;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_DIVU   = 5;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_REM    = 6;   ///PARAM DECODE
    constexpr int RV32_FUNCT3_REMU   = 7;   ///PARAM DECODE
}

#endif //KATHRYN_SRC_EXAMPLE_O3_ISAPARAM_H
