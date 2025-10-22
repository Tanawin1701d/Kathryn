//
// Created by tanawin on 26/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H

#include "broadCast.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    struct TagGen{
        BroadCast& bc;
        mReg(brdepth , BRDEPTH_LEN);
        mReg(tagreg  , SPECTAG_LEN);
        mWire(spTag1Result, SPECTAG_LEN);
        mWire(spTag2Result, SPECTAG_LEN);

        explicit TagGen(BroadCast& bc):
        bc(bc){
            brdepth.makeResetEvent();
            tagreg.makeResetEvent(1);

            spTag1Result = tagreg;
            spTag2Result = spTag1Result;
        }

        void onMisPred(opr& misTag){
            brdepth <<= 0;
            tagreg  <<= g(tagreg(SPECTAG_LEN-1), tagreg(0, SPECTAG_LEN-2));
            ///// the tag reg supposed to be the last tag that valid
            ///// so we shift right
        }

        void onSucPred(){
            brdepth <<=  (brdepth - 1);
        }

        opr& isAllGenble(opr& branchValid1, opr& branchValid2){

            opr& amtGen = (branchValid1.uext(2) +
                           branchValid2.uext(2)).uext(BRDEPTH_LEN);
            opr& amtFree = bc.isBrSuccPred().uext(BRDEPTH_LEN);

            return (brdepth + amtGen) <=
            (mOprVal("entNum", BRDEPTH_LEN, BRANCH_ENT_NUM) + amtFree);
        }

        opr& roundShift1(Reg& src){
            return g(src(0, SPECTAG_LEN-1), src(SPECTAG_LEN-1));
        }

        opr& roundShift2(Reg& src){
            return g(src(0, SPECTAG_LEN-2), src(SPECTAG_LEN-2, SPECTAG_LEN));
        }

        /////// isAllGenble should be used first
        std::pair<opr&, opr&> allocate(
            opr& branchValid1,Reg& spec1,
            opr& branchValid2,Reg& spec2){
            ///// allocate branch 1
            zif (branchValid1){
                spec1        <<= (brdepth != 0);
                spTag1Result   = roundShift1(tagreg);
                tagreg       <<= spTag1Result;
            }
            ///// allocate branch 2
            spec2 <<= (brdepth != 0) || (branchValid1);
            zif (branchValid2){
                zif (branchValid1){
                    spTag2Result = roundShift2(tagreg);
                }
                zelse{
                    spTag2Result = roundShift1(tagreg);
                }
                tagreg <<= spTag2Result;
            }
            //// update internal structure
            brdepth <<= ((((brdepth + branchValid1)
                                    + branchValid2)
                                    - bc.isBrSuccPred()));
            return {spTag1Result, spTag2Result};
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
