//
// Created by tanawin on 29/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H
#define KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H

#include "stage_struct.h"

namespace kathryn::o3{


    inline WireSlot decode_src_opr(RegSlot& dcd,
        opr* des_prev_idx, opr* is_des_prev_use,
        int src_idx, RegArch& ra){

        //// src Idx start from 1

        std::string rsIdx_i = "rsIdx_" + to_s(src_idx);
        std::string rsSel_i = "rsSel_" + to_s(src_idx);
        std::string rsUse_i = "rsUse_" + to_s(src_idx);

        std::string phyIdx_i  = "phyIdx_" + to_s(src_idx) ;
        std::string rsValid_i = "rsValid_"+ to_s(src_idx) ;

        ////// phy_idx equal to rrf_idx or data
        WireSlot result({phyIdx_i, rsSel_i, rsValid_i},
                         {DATA_LEN   ,   SRC_A_SEL_WIDTH, 1});

        result(rsSel_i) = dcd(rsSel_i);

        ////// we doesn't care if the src opr did not use

        ////// try to get data from the register architecture first
        RenamedData arf_ren = ra.arf.get_renamed_data(dcd(rsIdx_i));
        zif(arf_ren.busy){ ///// data is in flight
            PhyEntry phy_entry = ra.rrf.get_phy_entry(arf_ren.rrf_idx);
            zif (phy_entry.valid){ ///// it is finish can waiting to commit in rrf
                result(rsValid_i) = 1;
                result(phyIdx_i)  = phy_entry.data;
            }zelif(ra.bpp.is_by_passing(arf_ren.rrf_idx)){ ////// it is bypassing in the system
                result(rsValid_i) = 1;
                ra.bpp.ass_by_pass_data(result(phyIdx_i), arf_ren.rrf_idx);
            }zelse{ ////// no idea it may be in reservation station
                result(rsValid_i) = 0;
                result(phyIdx_i)  = arf_ren.rrf_idx.uext(DATA_LEN);
            }
        }zelse{ ///// data is cold in architecture regiter file
            result(rsValid_i) = 1;
            result(phyIdx_i)  = ra.arf.get_arf_data(dcd(rsIdx_i));
        }
        //////// everything will be override if it is prev_use
        if (is_des_prev_use != nullptr){
            assert(des_prev_idx != nullptr);
            zif(*is_des_prev_use){
                result(rsValid_i) = 0;
                result(phyIdx_i)  = des_prev_idx->uext(DATA_LEN);
            }
        }
        ////// register index is 0
        zif(dcd(rsIdx_i) == 0){
            result(rsValid_i) = 1;
            result(phyIdx_i)  = 0;
        }

        //////// everything will be override if it is prev_use
        zif (~dcd(rsUse_i)){
            result(rsValid_i) = 1;
        }

        return result;
    }



}

#endif //KATHRYN_SRC_EXAMPLE_O3_SRCOPR_H
