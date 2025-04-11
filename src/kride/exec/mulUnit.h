//
// Created by tanawin on 10/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_EXEC_MULUNIT_H
#define KATHRYN_SRC_KRIDE_EXEC_MULUNIT_H

#include "kride/krideParam.h"
#include "EUBase.h"
#include "mul.h"

namespace kathryn{

    struct MulUnit: EUBase{

        MulUnit(D_ALL& din, D_IO_RSV& dcIn):
        EUBase(din, dcIn){}

        void flow() override{
            Candidate& cd =  d.rsvMul.issueCand;
            Slot&      sl =  cd.detLogic;

            pip(EX_MUL){
                intrRstAndStart(d.shouldInv(sl));
                auto& res = MUL().exec(sl);
                writeReg(sl, res);
            }
        }


    };




}

#endif //KATHRYN_SRC_KRIDE_EXEC_MULUNIT_H
