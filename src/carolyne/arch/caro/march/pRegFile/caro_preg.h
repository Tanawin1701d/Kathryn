//
// Created by tanawin on 8/1/2025.
//

#ifndef src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
#define src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
#include "carolyne/arch/base/march/pRegFile/physicalRegFile_base.h"

namespace kathryn::carolyne::caro{

    constexpr int  PREGFILE_IDX_SIZE  = 8;
    constexpr int  PREG_WIDTH         = 32;
    constexpr char PHY_REGFILE_NAME[] = "int_preg";

    struct PRegFile: PhysicalRegFileBase{

        explicit PRegFile(){
            addRegGroup(PHY_REGFILE_NAME,
                        PREGFILE_IDX_SIZE,
                        PREG_WIDTH);
        }

    };


}

#endif //src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
