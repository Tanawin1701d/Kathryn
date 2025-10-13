//
// Created by tanawin on 26/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ISAPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_ISAPARAM_H

namespace kathryn::o3{
    // Basic parameters
    constexpr int INST_WIDTH      = 32;
    constexpr int REG_ADDR_WIDTH  = 5;
    constexpr int XPR_LEN         = 32;
    constexpr int DOUBLE_XPR_LEN  = 64;
    constexpr int LOG2_XPR_LEN    = 5;
    constexpr int SHAMT_WIDTH     = 5;

    constexpr int RV_NOP = 0b0010011;

    // Opcodes
    constexpr int RV32_LOAD   = 0b0000011;
    constexpr int RV32_STORE  = 0b0100011;
    constexpr int RV32_MADD   = 0b1000011;
    constexpr int RV32_BRANCH = 0b1100011;

    constexpr int RV32_LOAD_FP  = 0b0000111;
    constexpr int RV32_STORE_FP = 0b0100111;
    constexpr int RV32_MSUB     = 0b1000111;
    constexpr int RV32_JALR     = 0b1100111;

    constexpr int RV32_CUSTOM_0 = 0b0001011;
    constexpr int RV32_CUSTOM_1 = 0b0101011;
    constexpr int RV32_NMSUB    = 0b1001011;
    // 0b1101011 is reserved

    constexpr int RV32_MISC_MEM = 0b0001111;
    constexpr int RV32_AMO      = 0b0101111;
    constexpr int RV32_NMADD    = 0b1001111;
    constexpr int RV32_JAL      = 0b1101111;

    constexpr int RV32_OP_IMM   = 0b0010011;
    constexpr int RV32_OP       = 0b0110011;
    constexpr int RV32_OP_FP    = 0b1010011;
    constexpr int RV32_SYSTEM   = 0b1110011;

    constexpr int RV32_AUIPC    = 0b0010111;
    constexpr int RV32_LUI      = 0b0110111;
    // 0b1010111 is reserved
    // 0b1110111 is reserved

    // 0b0011011 is RV64-specific
    // 0b0111011 is RV64-specific
    constexpr int RV32_CUSTOM_2 = 0b1011011;
    constexpr int RV32_CUSTOM_3 = 0b1111011;

    // Arithmetic FUNCT3 encodings
    constexpr int RV32_FUNCT3_ADD_SUB  = 0;
    constexpr int RV32_FUNCT3_SLL      = 1;
    constexpr int RV32_FUNCT3_SLT      = 2;
    constexpr int RV32_FUNCT3_SLTU     = 3;
    constexpr int RV32_FUNCT3_XOR      = 4;
    constexpr int RV32_FUNCT3_SRA_SRL  = 5;
    constexpr int RV32_FUNCT3_OR       = 6;
    constexpr int RV32_FUNCT3_AND      = 7;

    // Branch FUNCT3 encodings
    constexpr int RV32_FUNCT3_BEQ  = 0;
    constexpr int RV32_FUNCT3_BNE  = 1;
    constexpr int RV32_FUNCT3_BLT  = 4;
    constexpr int RV32_FUNCT3_BGE  = 5;
    constexpr int RV32_FUNCT3_BLTU = 6;
    constexpr int RV32_FUNCT3_BGEU = 7;

    // MISC-MEM FUNCT3 encodings
    constexpr int RV32_FUNCT3_FENCE   = 0;
    constexpr int RV32_FUNCT3_FENCE_I = 1;

    // SYSTEM FUNCT3 encodings
    constexpr int RV32_FUNCT3_PRIV   = 0;
    constexpr int RV32_FUNCT3_CSRRW  = 1;
    constexpr int RV32_FUNCT3_CSRRS  = 2;
    constexpr int RV32_FUNCT3_CSRRC  = 3;
    constexpr int RV32_FUNCT3_CSRRWI = 5;
    constexpr int RV32_FUNCT3_CSRRSI = 6;
    constexpr int RV32_FUNCT3_CSRRCI = 7;

    // PRIV FUNCT12 encodings
    constexpr int RV32_FUNCT12_ECALL  = 0b000000000000;
    constexpr int RV32_FUNCT12_EBREAK = 0b000000000001;
    constexpr int RV32_FUNCT12_ERET   = 0b000100000000;

    // RV32M encodings
    constexpr int RV32_FUNCT7_MUL_DIV = 1;

    constexpr int RV32_FUNCT3_MUL    = 0;
    constexpr int RV32_FUNCT3_MULH   = 1;
    constexpr int RV32_FUNCT3_MULHSU = 2;
    constexpr int RV32_FUNCT3_MULHU  = 3;
    constexpr int RV32_FUNCT3_DIV    = 4;
    constexpr int RV32_FUNCT3_DIVU   = 5;
    constexpr int RV32_FUNCT3_REM    = 6;
    constexpr int RV32_FUNCT3_REMU   = 7;
}

#endif //KATHRYN_SRC_EXAMPLE_O3_ISAPARAM_H
