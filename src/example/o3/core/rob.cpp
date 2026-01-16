//
// Created by tanawin on 17/12/25.
//


#include "rob.h"
#include "storeBuf.h"

namespace kathryn::o3{

    void Rob::flow(){
        comPtr2 = comPtr + 1;
        comPtr <<= (comPtr + com1Status + com2Status);

        selectedEntry = com1Entry;
        zif(com2Status & com2Entry(storeBit)){
            ///// com2Status make sure that com1Status is not branch or store and current is ready to commit
            selectedEntry = com2Entry;
        }

        pip(pm.cm.sync){autoSync
            /////// commit the instruction
                ////// due to branch can do only one
            ////// we have to set commit commad
            opr& com1Cond = com1Entry(wbFin);
            opr& com2Cond = (com2Entry(wbFin)) & (~com1Entry(isBranch)) &
                            (~com1Entry(storeBit));
            ////// rrf commit to validate the com1Status/com2Status the commit status
            std::tie(com1Status, com2Status) =
                regArch.rrf.onCommit(comPtr, com1Cond, com2Cond);
            ////// arf commit
            regArch.arf.onCommit(
                //// com1
                com1Status & com1Entry(rdUse), comPtr ,
                com1Entry(rdIdx),
                regArch.rrf.getPhyData(comPtr),
                //// com2
                com2Status & com2Entry(rdUse), comPtr2,
                com2Entry(rdIdx),
                regArch.rrf.getPhyData(comPtr2)
            );

            zif ((com1Status & com1Entry(storeBit)) |
                 (com2Status & com2Entry(storeBit))){
                    storeBuf.onCommit();
            }
        }

    }
}