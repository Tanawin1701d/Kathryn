//
// Created by tanawin on 12/1/2025.
//

#ifndef src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
#define src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
#include <string>
#include "carolyne/arch/base/march/prfUnit/prfMetaBase.h"

namespace kathryn::carolyne{

    //////////// it map between architecture register group and
    //////////// physical register group
    constexpr char AP_GRP_UNSET[] = "UNSET";
    ////////// the slot in each rob
    struct APRegRobFieldMatch{
        ArchRegFileUTM* relatedArchRegFile = nullptr;
        int             idxInRob = -1; /////// -1 means kathryn can schedule where they match
        bool            isMAC    = false; ///// is memory access capability
        bool            isAddr   = false; ///// else is data;

        bool operator == (const APRegRobFieldMatch& rhs)const{
            return  (relatedArchRegFile == rhs.relatedArchRegFile) &&
                    (idxInRob           == rhs.idxInRob) &&
                    (isMAC              == rhs.isMAC) &&
                    (isAddr             == rhs.isAddr);
        }

    };

}

#endif //src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
