//
// Created by tanawin on 6/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REGFILE_CARO_ARCHREGFILE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REGFILE_CARO_ARCHREGFILE_H

#include "carolyne/arch/base/isa/regFile/archRegFile.h"
#include "carolyne/arch/caro/isa/param/param.h"



namespace kathryn::carolyne::caro{



    struct ArchRegFile: ArchRegFileBase{

        explicit ArchRegFile(){
            addRegGroup(ARCH_REGFILE_NAME,
                        REGFILE_IDX_SIZE,
                        REG_WIDTH);
        }

    };

}


#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_ISA_REGFILE_CARO_ARCHREGFILE_H
