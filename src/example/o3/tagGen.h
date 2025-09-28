//
// Created by tanawin on 26/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H

#include "kathryn.h"
#include "mpft.h"
#include "parameter.h"
#include "stageStruct.h"

namespace kathryn::o3{

    struct TagGen{

        mReg(brdepth , BRDEPTH_LEN);
        mReg(tagreg  , SPECTAG_LEN);

        TagGen(){
            brdepth.makeResetEvent(); //// TODO make it start to one
            tagreg.makeResetEvent();
        }


        opr& isAllGenble(opr& branchValid1, opr& branchValid2, BroadCast& bc){
            return (brdepth + branchValid1 + branchValid2) <=
            (mOprVal("entNum", SPECTAG_LEN, BRANCH_ENT_NUM) + bc.isBrSuccPred());
        }

        opr& roundShift1(Reg& src){
            return g(src(0, SPECTAG_LEN-2), src(SPECTAG_LEN-1));
        }

        opr& roundShift2(Reg& src){
            return g(src(0, SPECTAG_LEN-2), src(SPECTAG_LEN-2, SPECTAG_LEN));
        }

        void allocate(
            opr& branchValid1,Reg& spec1,Reg& spTag1,
            opr& branchValid2,Reg& spec2,Reg& spTag2,
            BroadCast& bc,
            Mpft& mpft){
            mWire(spTag1Result, SPECTAG_LEN);
            mWire(spTag2Result, SPECTAG_LEN);
            ///// allocate branch 1
            zif (branchValid1){
                spec1 <<= (brdepth != 0);
                spTag1Result = roundShift1(tagreg);
                spTag1 <<= spTag1Result;

            }
            ///// allocate branch 2
            zif (branchValid2){
                spec2 <<= (brdepth != 0) || (branchValid1);
                zif (branchValid1){ spTag2Result = roundShift2(tagreg);}
                zelse             { spTag2Result = roundShift1(tagreg);}
                spTag2 <<= spTag2Result;
            }

            //// update internal structure
            brdepth <<= (brdepth + branchValid1 + branchValid2 - bc.isBrSuccPred());
            zif (branchValid1 ^ branchValid2){
                tagreg <<= roundShift1(tagreg);
            }zelif(branchValid1 & branchValid2){
                tagreg <<= roundShift2(tagreg);
            }

            /////// update mpft
            ///
            ///
            mpft.onAddNew(branchValid1, spTag1Result,
                          branchValid2, spTag2Result);

        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
