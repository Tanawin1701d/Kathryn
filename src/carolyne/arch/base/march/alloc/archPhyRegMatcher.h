//
// Created by tanawin on 12/1/2025.
//

#ifndef src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
#define src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
#include <string>

namespace kathryn::carolyne{

    //////////// it map between architecture register group and
    //////////// physical register group
    struct APRegTypeMatch{
        std::string _archRegGrpName = "unname";
        std::string _phyRegGrpName  = "unname";

        bool operator == (const APRegTypeMatch& rhs)const{
            return (_archRegGrpName   == rhs._archRegGrpName) &&
                   (_phyRegGrpName    == rhs._phyRegGrpName);
        }

    };

}

#endif //src_carolyne_arch_base_march_alloc_ARCHPHYREGMATCHER_H
