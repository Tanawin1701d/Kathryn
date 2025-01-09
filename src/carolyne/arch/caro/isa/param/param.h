//
// Created by tanawin on 9/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_PARAM_PARAM_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_PARAM_PARAM_H

#include "model/hwComponent/abstract/Slice.h"

namespace kathryn::carolyne::caro{

    ///////// MOP PARAM
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

    ////////// ARCH REG FILE
    constexpr int REGFILE_IDX_SIZE     =  5; /// size is eq 32
    constexpr int REG_WIDTH            = 32;
    constexpr char ARCH_REGFILE_NAME[] = "int_reg";

    ////////// UOP PARAM
    constexpr char AOP_NAME[]      = "aop";
    constexpr char LOP_NAME[]      = "lop";
    constexpr char SOP_NAME[]      = "sop";
    constexpr char IOP_NAME[]      = "iop";
    constexpr int  XOP_IDENT_WIDTH = 7; /// the parameter that used to identify uop


}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_PARAM_PARAM_H
