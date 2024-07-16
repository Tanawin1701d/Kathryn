//
// Created by tanawin on 3/4/2567.
//

#ifndef KATHRYN_PARAMETER_H
#define KATHRYN_PARAMETER_H
#include "model/hwComponent/abstract/Slice.h"

namespace kathryn{

    namespace riscv {

        constexpr int   XLEN                     = 32;
        constexpr int   MEM_ADDR_IDX             = 32;
        constexpr int   MEM_ADDR_IDX_ACTUAL      = 28; /////256 megabyte
        constexpr int   REG_IDX                  =  5;
        constexpr int   UOP_WIDTH                =  7;
        constexpr int   AMT_REG                  = 32;

        constexpr int   AMT_DEC_SRC_REG          = 3;
        constexpr int   AMT_DEC_DES_REG          = 1;

        static const int   MEM_ADDR_IDX_ACTUAL_AL32 = MEM_ADDR_IDX_ACTUAL-2; /////256 megabyte
        static const Slice MEM_ADDR_SL              = {2, MEM_ADDR_IDX_ACTUAL};

    }

}

#endif //KATHRYN_PARAMETER_H
