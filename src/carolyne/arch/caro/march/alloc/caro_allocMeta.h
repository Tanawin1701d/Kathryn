//
// Created by tanawin on 8/1/2025.
//

#ifndef src_carolyne_arch_caro_march_alloc_CARO_ALLOC_H
#define src_carolyne_arch_caro_march_alloc_CARO_ALLOC_H

#include "carolyne/arch/base/march/alloc/allocMetaBase.h"
#include "carolyne/arch/caro/isa/regFile/caro_archRegFile.h"
#include "carolyne/arch/caro/march/pRegFile/caro_preg.h"
#include "carolyne/arch/caro/march/param/param.h"

namespace kathryn::carolyne::caro{

    struct AllocUTM: AllocUTM_Base{

        explicit AllocUTM()= default;

    };

}

#endif //src_carolyne_arch_caro_march_alloc_CARO_ALLOC_H
