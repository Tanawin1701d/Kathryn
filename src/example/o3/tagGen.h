//
// Created by tanawin on 26/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H

#include "kathryn.h"
#include "parameter.h"
#include "stageStruct.h"

namespace kathryn::o3{

    struct TagGen{
        BroadCast& bc;
        mReg(brdepth , BRDEPTH_LEN);
        mReg(tagreg  , SPECTAG_LEN);

        explicit TagGen(BroadCast& bc):
        bc(bc)
        {
            brdepth.makeResetEvent();
            tagreg.makeResetEvent(1);
        }

        void onMisPred(){
            brdepth <<= 0;
            tagreg  <<= 1;
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
            return g(src(0, SPECTAG_LEN-2), src(SPECTAG_LEN-1));
        }

        opr& roundShift2(Reg& src){
            return g(src(0, SPECTAG_LEN-2), src(SPECTAG_LEN-2, SPECTAG_LEN));
        }

        /////// isAllGenble should be used first
        std::pair<opr&, opr&> allocate(
            opr& branchValid1,Reg& spec1,
            opr& branchValid2,Reg& spec2){
            mWire(spTag1Result, SPECTAG_LEN);
            mWire(spTag2Result, SPECTAG_LEN);
            ///// allocate branch 1
            zif (branchValid1){
                spec1 <<= (brdepth != 0);
                spTag1Result = roundShift1(tagreg);
                //spTag1 <<= spTag1Result;

            }
            ///// allocate branch 2
            zif (branchValid2){
                spec2 <<= (brdepth != 0) || (branchValid1);
                zif (branchValid1){ spTag2Result = roundShift2(tagreg);}
                zelse             { spTag2Result = roundShift1(tagreg);}
                //spTag2 <<= spTag2Result;
            }

            //// update internal structure
            brdepth <<= (brdepth + branchValid1.uext(BRDEPTH_LEN) +
                                   branchValid2.uext(BRDEPTH_LEN) -
                                   bc.isBrSuccPred().uext(BRDEPTH_LEN));
            zif (branchValid1 ^ branchValid2){
                tagreg <<= roundShift1(tagreg);
            }zelif(branchValid1 & branchValid2){
                tagreg <<= roundShift2(tagreg);
            }

            return {spTag1Result, spTag2Result};

        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
