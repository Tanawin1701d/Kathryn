//
// Created by tanawin on 6/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REGFILE_CARO_ARCHREGFILE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REGFILE_CARO_ARCHREGFILE_H

#include "carolyne/arch/base/isa/regFile/archRegFile.h"
#include "carolyne/arch/caro/isa/param/param.h"



namespace kathryn::carolyne::caro{



    struct ArchRegFileUTM: ArchRegFileUTM_Base{

        explicit ArchRegFileUTM():
            ArchRegFileUTM_Base(CARO_REGFILE_IDX_SIZE,
                                CARO_REG_WIDTH,
                                ARCH_REGFILE_NAME){};
    };

}


#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REGFILE_CARO_ARCHREGFILE_H
