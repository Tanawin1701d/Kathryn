//
// Created by tanawin on 20/6/2024.
//

#include "logicGenBase.h"

#include <utility>
#include "gen/proxyHwComp/module/moduleGen.h"

namespace kathryn{


    LogicGenBase::LogicGenBase(ModuleGen*    mdGenMaster,
                               logicLocalCef cerf,
                               Assignable*   asb):
_mdGenMaster(mdGenMaster),
_cerf(std::move(cerf)),
_asb(asb){
    assert(mdGenMaster != nullptr);
}









}
