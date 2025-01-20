//
// Created by tanawin on 8/1/2025.
//

#ifndef src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
#define src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
#include "carolyne/arch/base/march/pRegFile/prfMetaBase.h"
#include "carolyne/arch/caro/march/param/param.h"


namespace kathryn::carolyne::caro{

    struct PRegFile: PhysicalRegFileBase{

        explicit PRegFile(){
            addRegGroup(PHY_REGFILE_NAME,
                        PREGFILE_IDX_SIZE,
                        PREG_WIDTH);
        }

    };


}

#endif //src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
