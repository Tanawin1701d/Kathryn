//
// Created by tanawin on 28/3/2025.
//

#ifndef KATHRYN_SRC_KRIDE_MPFT_MPFT_H
#define KATHRYN_SRC_KRIDE_MPFT_MPFT_H

#include "kride/incl.h"


namespace kathryn{

    struct Mpft: public Module{
        D_ALL d;
        Table mpftt;
        mVal(preS, SPEC_TAG_LEN, 1);

        Mpft(D_ALL& din): d(din), mpftt("mpftTable",
                                        {C_VALID, C_SPEC_PREF},
                                        {1      , SPEC_TAG_LEN},
                                        SPEC_TAG_SEL){}

        void flow() override{

            offer(MPFT_exec){
                ofc(EXEC){
                    for (int specIdx = 1; specIdx <= SPEC_TAG_LEN; specIdx++){
                        zif(d.exb.misPred | (d.exb.specTagRcv == specIdx)){
                            mpftt.get(specIdx).get(C_VALID) <<= 0;
                            mpftt.get(specIdx).get(C_SPEC_PREF) <<= 0;
                        }zelse{ //// pred Suc and idx not match --> delete some branch
                            mpftt.get(specIdx).get(C_SPEC_PREF) <<=
                                    (mpftt.get(specIdx).get(C_SPEC_PREF) &
                                    (~(preS << d.exb.specTagRcv)));
                        };
                    }
                }
                ofc(DP){
                    for (int specIdx = 1; specIdx <= SPEC_TAG_LEN; specIdx++) {
                        zif(mpftt.get(specIdx).get(C_VALID)){
                            mpftt.get(specIdx).get(C_SPEC_PREF) <<=
                                    mpftt.get(specIdx).get(C_SPEC_PREF) |
                                    d.dis.specTagPoolReq;
                        }
                    }
                }
            }


        }

    };

}

#endif //KATHRYN_SRC_KRIDE_MPFT_MPFT_H
