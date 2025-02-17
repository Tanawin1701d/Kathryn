//
// Created by tanawin on 17/2/2025.
//

#include "regAlloc.h"

namespace kathryn::carolyne{


    /**
     * ( master ) -> [oprRaw]
     * ( slave [case REG_REQ_PHY_ALLOC] ) -> [phyId]      (if case REG_REQ_ARCH_RENAME just rename it)
     * ( slave [case REG_REQ_ARCH_READ] ) -> [phyId]
     * (just return phyId)
     * */


    ///// oprRaw may be empty
    PopHST& RegAllocCtrl::createRegAllocTunnel(Slot oprRaw,
                                               DecodeSlotAnalyzer& dec,
                                               int idx, bool isSrc){
        OprTypeBase* oprType = dec.
        ALLOC_INFO allocinfo = oprType->getAllocInfo();
        crlAss(isThere(allocinfo.regAllocOption, REG_OPT::REG_NO_REQ), "there is no opr to request");

        std::string

        createHST()

        return ;
    }
}