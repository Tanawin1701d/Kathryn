//
// Created by tanawin on 8/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_EXEC_INTUNIT_H
#define KATHRYN_SRC_KRIDE_EXEC_INTUNIT_H

#include "alu.h"
#include "kride/krideParam.h"
#include "EUBase.h"


namespace kathryn{

    struct IntUnit: EUBase{

        int idx;
        mWire(opr1, DATA_LEN);
        mWire(opr2, DATA_LEN);

        IntUnit(D_ALL& din, D_IO_RSV& dcIn, int idxIn):
        EUBase(din, dcIn), idx(idxIn){}

        void flow() override{
            Candidate& cd = d.rsvInt[idx].issueCand;
            Slot& sl = cd.detLogic;
            srcASel(opr1, sl);
            srcBSel(opr2, sl);

            //////// todo check about spec bit

            pip(EX_INTP + its(idx)){
                intrRstAndStart(d.shouldInv(sl));
                auto& res = ALU().exec(sl.at("aluOp"), opr1, opr2);
                writeReg(sl, res);
            }
        }
    };


}

#endif //KATHRYN_SRC_KRIDE_EXEC_INTUNIT_H
