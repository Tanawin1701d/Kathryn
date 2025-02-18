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
    PopHST& RegAllocCtrl::createRegAllocTunnel(DecodeSlotAnalyzer& dec,
                                               int idx, bool isSrc){
        ////// get operand information
        OprTypeBase*          oprType = dec.getOprType(idx, isSrc);
        ALLOC_INFO            allocinfo = oprType->getAllocInfo();
        PhyRegFileUTM_Base*  relatedPhyRegFile = allocinfo.relatedPhyRegFile;
        crlAss(isThere(allocinfo.regAllocOption, REG_OPT::REG_NO_REQ), "there is no opr to request");

        ////// create tunnel
        std::string tunnelName   = getTunnelName(idx, isSrc);
        RowMeta     sendSlotMeta = dec.getOprRawData(idx, isSrc).getMeta(); ////// opr_raw_[src/des]
        RowMeta     recvSlotMeta = RowMeta({SLOT_F_ALLOC_PHYID}, {32});
        ///////////// in case requireData (architecture )
        if (isThere(allocinfo.regAllocOption, REG_OPT::REG_REQ_ARCH_READ)){
            recvSlotMeta += RowMeta({SLOT_F_ALLOC_DATA_VALID, SLOT_F_ALLOC_DATA},
                                    {1, relatedPhyRegFile->getRegWidth()});
        }

        PopHST& newHst = createHST(tunnelName, sendSlotMeta, recvSlotMeta);
        return newHst;
    }

    std::string RegAllocCtrl::getTunnelName(int idx, bool isSrc){
        std::string result = isSrc ? TNF_ALLOCREG_SRC: TNF_ALLOCREG_DES;
        result = result + "_" + std::to_string(idx);
        return result;
    }

}