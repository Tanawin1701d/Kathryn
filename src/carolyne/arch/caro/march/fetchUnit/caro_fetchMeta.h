//
// Created by tanawin on 8/1/2025.
//

#ifndef src_carolyne_arch_caro_march_fetchUnit_CARO_FETCHMETA_H
#define src_carolyne_arch_caro_march_fetchUnit_CARO_FETCHMETA_H

#include "carolyne/arch/base/march/fetchUnit/fetchMetaBase.h"
#include "carolyne/arch/caro/march/param/param.h"

namespace kathryn::carolyne::caro{

    struct FetchUTM: FetchUTM_Base{

        explicit FetchUTM(): FetchUTM_Base(PC_BIT_WIDTH, INSTR_BIT_WIDTH){}

    };
}

#endif //src_carolyne_arch_caro_march_fetchUnit_CARO_FETCHMETA_H
