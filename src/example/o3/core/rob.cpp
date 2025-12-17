//
// Created by tanawin on 17/12/25.
//


#include "rob.h"
#include "fetch.h"
#include "storeBuf.h"

namespace kathryn::o3{
    void Rob::flow(){
        comPtr2 = comPtr + 1;
        comPtr <<= (comPtr + com1Status + com2Status);
        ////// we have to set commit commad

        pip(pm.cm.sync){autoSync
            /////// commit the instruction
                ////// due to branch can do only one
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
            ///// handle branch
            zif(com1Status){
                zif(com1Entry(isBranch)){
                    fetchMod.onBranchCommit();
                }
                zif(com1Entry(storeBit)){
                    storeBuf.onCommit();
                }
            }
        }

    }
}