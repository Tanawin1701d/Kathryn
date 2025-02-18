//
// Created by tanawin on 17/2/2025.
//

#ifndef KATHRYN_carolyne_model_base_alloc_regAlloc_H
#define KATHRYN_carolyne_model_base_alloc_regAlloc_H

#include <utility>

#include "lib/hw/popper/popMod.h"
#include "carolyne/arch/base/march/repo/repo.h"
#include "carolyne/arch/base/isa/repo/repo.h"
#include "carolyne/model/base/coreArgs.h"
#include "carolyne/model/base/decode/decodeSlotAnal.h"

namespace kathryn::carolyne{

    struct RegAllocCtrl: PopTunnelable{
        int numberRegAllocTunnel = 0;
        std::string _prefixName = "unname";

        explicit RegAllocCtrl(std::string  prefixName, PopTunnelMng* tunnelMng):
        PopTunnelable(tunnelMng),
        _prefixName(std::move(prefixName)){}
                                          ///// oprRaw is only opr_raw_src or opr_raw_des
        PopHST& createRegAllocTunnel(DecodeSlotAnalyzer& dec,
                                     int idx, bool isSrc);

        static std::string getTunnelName(int idx, bool isSrc);




    };

}


#endif //KATHRYN_carolyne_model_base_alloc_regAlloc_H
