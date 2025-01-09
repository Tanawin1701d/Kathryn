//
// Created by tanawin on 8/1/2025.
//

#ifndef src_carolyne_arch_caro_march_param_PARAM_H
#define src_carolyne_arch_caro_march_param_PARAM_H

namespace kathryn::carolyne::caro{

    //////// FETCH UNIT PARAMETER
    constexpr int INSTR_BIT_WIDTH = 32;
    constexpr int PC_BIT_WIDTH    = 32;

    //////// ALLOC UNIT PARAMETER

    //////// PHYSICAL REG UNIT PARAMETER
    constexpr int  PREGFILE_IDX_SIZE  = 8;
    constexpr int  PREG_WIDTH         = 32;
    constexpr char PHY_REGFILE_NAME[] = "int_preg";


}

#endif //src_carolyne_arch_caro_march_param_PARAM_H
