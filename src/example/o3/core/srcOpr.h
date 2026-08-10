//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H
#define KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H

#include "stageStruct.h"

namespace kathryn::o3{


    inline WireSlot decodeSrcOpr(RegSlot& dcd,   ///CTRL_HC+DATA_HC DISPATCH
        opr* desPrevIdx, opr* isDesPrevUse,      ///CTRL_HC DISPATCH
        int srcIdx, RegArch& ra){                ///CTRL_HC+DATA_HC DISPATCH

        //// src Idx start from 1

        std::string rsIdx_i = "rsIdx_" + toS(srcIdx);   ///HLH DISPATCH
        std::string rsSel_i = "rsSel_" + toS(srcIdx);   ///HLH DISPATCH
        std::string rsUse_i = "rsUse_" + toS(srcIdx);   ///HLH DISPATCH

        std::string phyIdx_i  = "phyIdx_" + toS(srcIdx) ;   ///HLH DISPATCH
        std::string rsValid_i = "rsValid_"+ toS(srcIdx) ;   ///HLH DISPATCH

        ////// phyIdx equal to rrfIdx or data
        WireSlot result({phyIdx_i, rsSel_i, rsValid_i},        ///CTRL_HWD+DATA_HWD DISPATCH
                         {DATA_LEN   ,   SRC_A_SEL_WIDTH, 1}); ///PARAM DISPATCH

        result(rsSel_i) = dcd(rsSel_i);   ///DATA_DT DISPATCH

        ////// we doesn't care if the src opr did not use

        ////// try to get data from the register architecture first
        RenamedData arfRen = ra.arf.getRenamedData(dcd(rsIdx_i));                             ///CTRL_DT DISPATCH
        zif(arfRen.busy){ ///// data is in flight                                             ///CTRL_CL DISPATCH
            PhyEntry phyEntry = ra.rrf.getPhyEntry(arfRen.rrfIdx);                            ///CTRL_DT+DATA_DT DISPATCH
            zif (phyEntry.valid){ ///// it is finish can waiting to commit in rrf             ///CTRL_CL DISPATCH
                result(rsValid_i) = 1;                                                        ///CTRL_DT DISPATCH
                result(phyIdx_i)  = phyEntry.data;                                            ///DATA_DT DISPATCH
            }zelif(ra.bpp.isByPassing(arfRen.rrfIdx)){ ////// it is bypassing in the system   ///CTRL_CL DISPATCH
                result(rsValid_i) = 1;                                                        ///CTRL_DT DISPATCH
                ra.bpp.assByPassData(result(phyIdx_i), arfRen.rrfIdx);                        ///CTRL_HC+DATA_HC DISPATCH
            }zelse{ ////// no idea it may be in reservation station
                result(rsValid_i) = 0;                                                        ///CTRL_DT DISPATCH
                result(phyIdx_i)  = arfRen.rrfIdx.uext(DATA_LEN);                             ///CTRL_CL DISPATCH
            }
        }zelse{ ///// data is cold in architecture regiter file
            result(rsValid_i) = 1;                                                            ///CTRL_DT DISPATCH
            result(phyIdx_i)  = ra.arf.getArfData(dcd(rsIdx_i));                              ///DATA_DT DISPATCH
        }
        //////// everything will be override if it is prevUse
        if (isDesPrevUse != nullptr){                                                         ///HLH DISPATCH
            assert(desPrevIdx != nullptr);
            zif(*isDesPrevUse){                                                               ///CTRL_CL DISPATCH
                result(rsValid_i) = 0;                                                        ///CTRL_DT DISPATCH
                result(phyIdx_i)  = desPrevIdx->uext(DATA_LEN);                               ///CTRL_CL DISPATCH
            }
        }
        ////// register index is 0
        zif(dcd(rsIdx_i) == 0){                                                               ///CTRL_CL DISPATCH
            result(rsValid_i) = 1;                                                            ///CTRL_DT DISPATCH
            result(phyIdx_i)  = 0;                                                            ///DATA_DT DISPATCH
        }

        //////// everything will be override if it is prevUse
        zif (~dcd(rsUse_i)){                                                                  ///CTRL_CL DISPATCH
            result(rsValid_i) = 1;                                                            ///CTRL_DT DISPATCH
        }

        return result;                                                                        ///DATA_HC DISPATCH
    }



}

#endif //KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H
