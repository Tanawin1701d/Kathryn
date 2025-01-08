//
// Created by tanawin on 2/1/2025.
//

#ifndef src_carolyne_arch_base_march_rsvUnit_RSVMETABASE_H
#define src_carolyne_arch_base_march_rsvUnit_RSVMETABASE_H

#include "carolyne/arch/base/march/execUnit/execMetaBase.h"

namespace kathryn::carolyne{

    struct RsvUTM_Base{

        std::vector<ExecRsvUnitMatch> execUnits;

        void addExecUnit(ExecUnitTypeMeta* execUnitType, int amt){
            crlAss(execUnitType != nullptr, "add add execunitType as nullptr too execUnits");
            execUnits.emplace_back(execUnitType, amt);
        }

        [[nodiscard]]
        std::vector<ExecRsvUnitMatch> getExecUnitMeta()const{return execUnits;}


    };

}

#endif //src_carolyne_arch_base_march_rsvUnit_RSVMETABASE_H
