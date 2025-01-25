//
// Created by tanawin on 8/1/2025.
//

#ifndef src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
#define src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
#include "carolyne/arch/base/march/prfUnit/prfMetaBase.h"
#include "carolyne/arch/caro/march/param/param.h"


namespace kathryn::carolyne::caro{

    struct PRegFile: PhyRegFileUTM_Base{
        explicit PRegFile():
        PhyRegFileUTM_Base(PREGFILE_IDX_SIZE,
                           PREG_WIDTH,
                           PHY_REGFILE_NAME){}
    };


}

#endif //src_carolyne_arch_caro_march_pRegFile_CARO_PREG_H
