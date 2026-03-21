//
// Created by tanawin on 17/12/25.
//


#include "rob.h"
#include "store_buf.h"

namespace kathryn::o3{

    void Rob::flow(){
        com_ptr2 = com_ptr + 1;
        com_ptr <<= (com_ptr + com1Status + com2Status);

        pip(pm.sync_cm){auto_sync ///CTRL ROB
            /////// commit the instruction
                ////// due to branch can do only one
            ////// we have to set commit commad
            opr& com1Cond = com1Entry(wb_fin);
            opr& com2Cond = (com2Entry(wb_fin)) & (~com1Entry(is_branch)) &
                            (~com1Entry(store_bit));
            ////// rrf commit to validate the com1Status/com2Status the commit status
            std::tie(com1Status, com2Status) =
                reg_arch.rrf.on_commit(com_ptr, com1Cond, com2Cond);
            ////// arf commit
            reg_arch.arf.on_commit(
                //// com1
                com1Status & com1Entry(rd_use), com_ptr ,
                com1Entry(rd_idx),
                reg_arch.rrf.get_phy_data(com_ptr),
                //// com2
                com2Status & com2Entry(rd_use), com_ptr2,
                com2Entry(rd_idx),
                reg_arch.rrf.get_phy_data(com_ptr2)
            );

            zif ((com1Status & com1Entry(store_bit)) |
                 (com2Status & com2Entry(store_bit))){
                    store_buf.on_commit();
            }
        }

    }
}