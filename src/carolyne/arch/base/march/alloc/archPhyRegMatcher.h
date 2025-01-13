//
// Created by tanawin on 12/1/2025.
//

#ifndef src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
#define src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
#include <string>

namespace kathryn::carolyne{

    //////////// it map between architecture register group and
    //////////// physical register group
    constexpr char AP_GRP_UNSET[] = "UNSET";
    struct APRegTypeMatch{
        std::string _archRegGrpName = AP_GRP_UNSET;
        std::string _phyRegGrpName  = AP_GRP_UNSET;

        bool operator == (const APRegTypeMatch& rhs)const{
            return (_archRegGrpName   == rhs._archRegGrpName) &&
                   (_phyRegGrpName    == rhs._phyRegGrpName);
        }
    };


    ////////// the slot in each rob
    struct APRegRobFieldMatch: APRegTypeMatch{
        int  idxInRob = -1;    /////// -1 means kathryn can schedule where they match
        bool isMAC    = false; ///// is memory access capability
        bool isAddr   = false; ///// else is data;
    };

}

#endif //src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
