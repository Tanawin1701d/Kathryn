//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H
#define KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H

#include "kathryn.h"
#include "parameter.h"
#include "stageStruct.h"

namespace kathryn::o3{


    inline WireSlot decodeSrcOpr(RegSlot& dcd,
        opr* desPrevIdx, opr* isDesPrevUse,
        int srcIdx, RegArch& ra){

        //// src Idx start from 1

        std::string rsIdx_i = "rsIdx_" + toS(srcIdx);
        std::string rsSel_i = "rsSel_" + toS(srcIdx);
        std::string rsUse_i = "rsUse_" + toS(srcIdx);

        std::string phyIdx_i  = "phyIdx_" + toS(srcIdx) ;
        std::string rsValid_i = "rsValid_"+ toS(srcIdx) ;

        ////// phyIdx equal to rrfIdx or data
        WireSlot result({phyIdx_i, rsSel_i, rsValid_i},
                         {DATA_LEN   ,   SRC_A_SEL_WIDTH, 1});

        result(rsSel_i) = dcd(rsSel_i);

        zif(dcd(rsUse_i)){
            RenamedData arfRen = ra.arf.getRenamedData(dcd(rsIdx_i));
            zif(arfRen.busy){ ///// data should be in rrf/ may commit or not
                PhyEntry phyEntry = ra.rrf.getPhyEntry(arfRen.rrfIdx);
                zif (phyEntry.valid){
                    result(rsValid_i) = 1;
                    result(phyIdx_i)  = phyEntry.data;
                }zelif(ra.bpp.isByPassing(arfRen.rrfIdx)){
                    result(rsValid_i) = 1;
                    result(phyIdx_i)  = ra.bpp.getByPassData(arfRen.rrfIdx);
                }zelse{
                    result(rsValid_i) = 0;
                    result(phyIdx_i)  = arfRen.rrfIdx.uext(DATA_LEN);
                }
            }zelse{
                result(rsValid_i) = 1;
                result(phyIdx_i)  = ra.arf.getArfData(dcd(rsIdx_i));
            }
            //////// everything will be override if it is prevUse
            if (isDesPrevUse != nullptr){
                assert(desPrevIdx != nullptr);
                zif(*isDesPrevUse){
                    result(rsValid_i) = 0;
                    result(phyIdx_i)  = desPrevIdx->uext(DATA_LEN);
                }
            }
        }zelse{
            result(rsValid_i) = 1;
            result(phyIdx_i) = 0;
        }
        return result;
    }



}

#endif //KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H
