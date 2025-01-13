//
// Created by tanawin on 8/1/2025.
//

#ifndef src_carolyne_arch_caro_march_robUnit_CARO_ROBMETA_H
#define src_carolyne_arch_caro_march_robUnit_CARO_ROBMETA_H

#include "carolyne/arch/base/march/robUnit/robMetaBase.h"
#include "carolyne/arch/caro/march/param/param.h"


namespace kathryn::carolyne::caro{

    struct RobUTM: RobUTM_Base{

        explicit RobUTM(ArchRegFile* archRegFile,
                        PRegFile* phyRegFile):
        RobUTM_Base(archRegFile,phyRegFile){
            
        }
    };

        //////// we will add more element

};


#endif //src_carolyne_arch_caro_march_robUnit_CARO_ROBMETA_H
