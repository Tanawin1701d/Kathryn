//
// Created by tanawin on 27/3/2025.
//

#ifndef KATHRYN_SRC_KRIDE_REG_RRFFINDER_RRFFINDER_H
#define KATHRYN_SRC_KRIDE_REG_RRFFINDER_RRFFINDER_H

#include "kathryn.h"
#include "kride/krideParam.h"
#include "kride/incl.h"



namespace kathryn{

    struct RrfFinder: Module{
        D_ALL& d;
        mReg(freeAmt, RRF_SEL+1);
        mReg(rrfPtr, RRF_SEL);
        mVal(RRF_AMT, RRF_SEL+1, AMT_PREG);

        RrfFinder(D_ALL& din): d(din){};

        void flow() override{

            offer(RF_FINDER){
                ofcc(EXEC_BRANCH, d.exb.misPred){
                    rrfPtr  <<= d.exb.rrfIdxRcv;
                    zif(d.exb.rrfIdxRcv >= d.com.nxtCommitIdx){
                        freeAmt <<= RRF_AMT - (d.exb.rrfIdxRcv - d.com.nxtCommitIdx);
                    }zelse{
                        freeAmt <<= (d.com.nxtCommitIdx - d.exb.rrfIdxRcv);
                    };
                }
                ofcc(DP, (freeAmt + d.com.comAmt) >= 0){
                    freeAmt <<= freeAmt - d.dis.rmAmt;
                    rrfPtr  <<= (rrfPtr + d.dis.rmAmt(0, RRF_SEL));
                }
                ofc(COMMIT){
                    freeAmt <<= freeAmt + d.com.comAmt;
                }
            }

        }



    };

}

#endif //KATHRYN_SRC_KRIDE_REG_RRFFINDER_RRFFINDER_H
