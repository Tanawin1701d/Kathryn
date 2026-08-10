//
// Created by tanawin on 17/12/25.
//


#include "rob.h"
#include "storeBuf.h"

namespace kathryn::o3{

    void Rob::flow(){                                    ///HLH ROB
        comPtr2 = comPtr + 1;                            ///CTRL_CL ROB
        comPtr <<= (comPtr + com1Status + com2Status);   ///CTRL_CL ROB

        pip(pm.sync_cm){autoSync                         ///CTRL_HWD+CTRL_CL ROB
            /////// commit the instruction
                ////// due to branch can do only one
            ////// we have to set commit commad
            opr& com1Cond = com1Entry(wbFin);                               ///CTRL_DT ROB
            opr& com2Cond = (com2Entry(wbFin)) & (~com1Entry(isBranch)) &   ///CTRL_CL ROB
                            (~com1Entry(storeBit));                         ///CTRL_CL ROB
            ////// rrf commit to validate the com1Status/com2Status the commit status
            std::tie(com1Status, com2Status) =                    ///HLH ROB
                regArch.rrf.onCommit(comPtr, com1Cond, com2Cond); ///CTRL_HC ROB
            ////// arf commit
            regArch.arf.onCommit(                       ///CTRL_HC ROB
                //// com1
                com1Status & com1Entry(rdUse), comPtr , ///CTRL_HC ROB
                com1Entry(rdIdx),                       ///DATA_HC ROB
                regArch.rrf.getPhyData(comPtr),         ///DATA_HC ROB
                //// com2
                com2Status & com2Entry(rdUse), comPtr2, ///CTRL_HC ROB
                com2Entry(rdIdx),                       ///DATA_HC ROB
                regArch.rrf.getPhyData(comPtr2)         ///DATA_HC ROB
            );

            zif ((com1Status & com1Entry(storeBit)) |   ///CTRL_CL ROB
                 (com2Status & com2Entry(storeBit))){   ///CTRL_CL ROB
                    storeBuf.onCommit();                ///CTRL_HC ROB
            }
        }

    }
}
