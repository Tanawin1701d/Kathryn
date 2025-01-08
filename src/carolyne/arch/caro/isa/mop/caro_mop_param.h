//
// Created by tanawin on 6/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_MOP_CARO_MOP_PARAM_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_MOP_CARO_MOP_PARAM_H

#include "model/hwComponent/abstract/Slice.h"

namespace kathryn::carolyne::caro{

    constexpr int  MOP_WIDTH    = 32;

    constexpr Slice FUNC7_POS   = {4, 4+7};
    constexpr Slice RD_POS      = {11, 11+5};
    constexpr Slice R1_POS      = {16, 16+5};
    constexpr Slice R2_POS      = {21, 21+5};
    constexpr Slice RI_POS      = {16, 16+16};

    constexpr Slice R1_POS_SOP  = {11, 11+5};
    constexpr Slice R2_POS_SOP  = {16, 16+5};


    constexpr char A_MOP_NAME[] = "a_mop";
    constexpr char L_MOP_NAME[] = "l_mop";
    constexpr char S_MOP_NAME[] = "s_mop";
    constexpr char I_MOP_NAME[] = "i_mop";
    constexpr char B_MOP_NAME[] = "b_mop";
}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_MOP_CARO_MOP_PARAM_H
