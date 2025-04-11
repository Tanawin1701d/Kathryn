//
// Created by tanawin on 10/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_EXEC_MUL_H
#define KATHRYN_SRC_KRIDE_EXEC_MUL_H

#include "kride/incl.h"

namespace kathryn{

    struct MUL{
        mWire(opr1, DATA_LEN*2);
        mWire(opr2, DATA_LEN*2);

        static void selSign(Operable& desOpr , Operable& sel,
                     Operable& signOpr, Operable& unSignOpr){
            zif(sel){desOpr = signOpr;}
            zelse   {desOpr = unSignOpr;}
        }

        Operable& exec(Slot& slot){
            mWire(res , DATA_LEN);
            ///// prepare the variable
            Operable& mos = slot.at("mdOutSel");
            selSign(opr1, slot.at("isMd1Sig"),
                slot.at("src1").sext(DATA_LEN*2),
                slot.at("src1").uext(DATA_LEN*2));
            selSign(opr2, slot.at("isMd2Sig"),
                slot.at("src2").sext(DATA_LEN*2),
                slot.at("src2").uext(DATA_LEN*2));
            ///// do multiplier and have the mux
            auto& mul = opr1*opr2;
            zif(mos == MD_OUT_LO){ res = mul(0       , DATA_LEN);  }
            zif(mos == MD_OUT_HI){ res = mul(DATA_LEN, DATA_LEN*2);}

            return res;
        }

    };



}

#endif //KATHRYN_SRC_KRIDE_EXEC_MUL_H
