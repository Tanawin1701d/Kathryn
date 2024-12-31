//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_execUnit_EXECMETABASE_H
#define src_carolyne_arch_base_march_execUnit_EXECMETABASE_H
#include <vector>
#include "carolyne/arch/base/isa/uop/opr_base.h"
#include "carolyne/arch/base/util/rowMeta.h"

namespace kathryn{
    namespace carolyne{


        struct ExecUnitMeta{
            int _execUnitType  = -1;
            int _execUnitIdx   = -1;
            int _belongToRsvId = -1;


            ExecUnit(int execUnitType, int execUnitIdx, int belongToRobId):
            _execUnitType (execUnitType),
            _execUnitIdx  (execUnitIdx),
            _belongToRobId(belongToRobId){}

        };

    }
}

#endif //src_carolyne_arch_base_march_execUnit_EXECMETABASE_H
