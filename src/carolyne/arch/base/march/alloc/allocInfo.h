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
    ///
    /// how rob deal with physical register file
    enum class ROB_OPT: int{
        ROB_NO_REQ                = 0,
        ROB_REQ_PHY_ID            = 1 << 0,
        ROB_REQ_ARCH_ID           = 1 << 1, //////// update policy
        ROB_REQ_UPT_ARCH          = 1 << 2, ///// update that arch register is now be the present commit and link to phy id
        ROB_REQ_UPT_ARCH_FROM_EXT = 1 << 3
    };

    enum class REG_OPT: int{
        REG_NO_REQ          = 0,
        REG_REQ_PHY_ALLOC   = 1 << 0, ////// require physical register file allocation
        REG_REQ_ARCH_READ   = 1 << 1, ////// require read data from architecture file -> but return physical id
        REG_REQ_ARCH_RENAME = 1 << 2  ////// require the allocated from first REG_REQ_PHY_ALLOC rename
    };

    /** helper function for rob_opt and reg_opt*/
    inline ROB_OPT operator | (ROB_OPT lhs, ROB_OPT rhs){
        return static_cast<ROB_OPT>(static_cast<int>(lhs) | static_cast<int>(rhs));
    }
    inline bool isThere(ROB_OPT opt, ROB_OPT checkOpt){
        return static_cast<bool>(static_cast<int>(opt) & static_cast<int>(checkOpt));
    }

    inline void verifyRobOpt (ROB_OPT rob_opt){

        crlAss(isThere(rob_opt, ROB_OPT::ROB_REQ_UPT_ARCH) ^
            isThere(rob_opt, ROB_OPT::ROB_REQ_UPT_ARCH_FROM_EXT),
            " rob alloc option cannot have both UPT_ARCH AND UPT_ARCH_FROM_EXT"
        );
    }

    inline REG_OPT operator | (REG_OPT lhs, REG_OPT rhs){
        return static_cast<REG_OPT>(static_cast<int>(lhs) | static_cast<int>(rhs));
    }
    inline bool isThere(REG_OPT opt, REG_OPT checkOpt){
        return static_cast<bool>(static_cast<int>(opt) & static_cast<int>(checkOpt));
    }
    //////////////////////////////////////////////////////

    struct ALLOC_INFO{
        ArchRegFileUTM_Base* relatedArchRegFile    = nullptr;
        PhyRegFileUTM_Base*  relatedPhyRegFile = nullptr;
        ROB_OPT             robUpdateOption{};
        REG_OPT             regAllocOption {};

        bool operator == (const ALLOC_INFO& rhs)const{
            return ( relatedArchRegFile  == rhs.relatedArchRegFile  ) &&
                   ( relatedPhyRegFile   == rhs.relatedPhyRegFile   ) &&
                   ( robUpdateOption == rhs.robUpdateOption         ) &&
                   ( regAllocOption  == rhs.regAllocOption          );
        }

    };

}

#endif //src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
