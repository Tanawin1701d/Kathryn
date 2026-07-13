//
// Created by tanawin on 26/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H

#include "broadCast.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    struct TagGen{                          ///MD TAG
        BroadCast& bc;                      ///CTRL_HC TAG
        mReg(brdepth , BRDEPTH_LEN);        ///CTRL_HWD TAG
        mReg(tagreg  , SPECTAG_LEN);        ///CTRL_HWD TAG
        mWire(spTag1Result, SPECTAG_LEN);   ///CTRL_HWD TAG
        mWire(spTag2Result, SPECTAG_LEN);   ///CTRL_HWD TAG

        explicit TagGen(BroadCast& bc):   ///CTRL_HC TAG
        bc(bc){                           ///CTRL_HC TAG
            brdepth.makeResetEvent();     ///CTRL_DT TAG
            tagreg.makeResetEvent(1);     ///CTRL_DT TAG
        }

        void onMisPred(opr& misTag){                                   ///CTRL_HC TAG
            brdepth <<= 0;                                             ///CTRL_DT TAG
            tagreg  <<= gr(misTag.sl(0), misTag.sl(1, SPECTAG_LEN));   ///CTRL_CL TAG
            ///// the tag reg supposed to be the last tag that valid and shift right 1
        }

        void onSucPred(){                 ///HLH TAG
            brdepth <<=  (brdepth - 1);   ///CTRL_CL TAG
        }

        opr& isAllGenble(opr& branchValid1, opr& branchValid2){     ///CTRL_HC TAG

            opr& amtGen = (branchValid1.uext(2) +                   ///CTRL_CL TAG
                           branchValid2.uext(2)).uext(BRDEPTH_LEN); ///CTRL_CL TAG
            opr& amtFree = bc.isBrSuccPred().uext(BRDEPTH_LEN);     ///CTRL_CL TAG

            return (brdepth + amtGen) <=                                ///CTRL_CL TAG
            (mOprVal("entNum", BRDEPTH_LEN, BRANCH_ENT_NUM) + amtFree); ///CTRL_CL TAG
        }

        opr& roundShift1(Reg& src){                                ///CTRL_HC TAG
            return g(src(0, SPECTAG_LEN-1), src(SPECTAG_LEN-1));   ///CTRL_CL TAG
        }

        opr& roundShift2(Reg& src){                                             ///CTRL_HC TAG
            return g(src(0, SPECTAG_LEN-2), src(SPECTAG_LEN-2, SPECTAG_LEN));   ///CTRL_CL TAG
        }

        /////// isAllGenble should be used first
        std::pair<opr&, opr&> allocate(                                            ///CTRL_HC TAG
            opr& branchValid1,Reg& spec1,                                          ///CTRL_HC TAG
            opr& branchValid2,Reg& spec2){                                         ///CTRL_HC TAG
            spec1 <<= (brdepth != 0);                                              ///CTRL_CL TAG
            spTag1Result = mux(branchValid1,roundShift1(tagreg), tagreg);          ///CTRL_CL TAG
            spec2 <<= (brdepth != 0) || (branchValid1);                            ///CTRL_CL TAG
            spTag2Result = mux(branchValid1 & branchValid2, roundShift2(tagreg),   ///CTRL_CL TAG
                           mux(branchValid2, roundShift1(tagreg), spTag1Result///// branchValid1 is false ///CTRL_CL TAG
                            ));
            tagreg <<= spTag2Result;   ///CTRL_DT TAG
            brdepth <<= ((((brdepth + branchValid1) + branchValid2) ///CTRL_CL TAG
                                    - bc.isBrSuccPred())); ///CTRL_CL TAG
            return {spTag1Result, spTag2Result};   ///CTRL_HC TAG
        }

            // ///// allocate branch 1
            // zif (branchValid1){
            //     spec1        <<= (brdepth != 0);
            //     spTag1Result   = roundShift1(tagreg);
            //     tagreg       <<= spTag1Result;
            // }
            // ///// allocate branch 2
            // spec2 <<= (brdepth != 0) || (branchValid1);
            // zif (branchValid2){
            //     zif (branchValid1){
            //         spTag2Result = roundShift2(tagreg);
            //     }
            //     zelse{
            //         spTag2Result = roundShift1(tagreg);
            //     }
            //     tagreg <<= spTag2Result;
            // }
            //// update internal structure


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
