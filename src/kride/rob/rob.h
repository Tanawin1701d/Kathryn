//
// Created by tanawin on 16/4/2025.
//

#ifndef KATRHYN_SRC_KRIDE_ROB_H
#define KATRHYN_SRC_KRIDE_ROB_H

#include "kride/incl.h"

namespace kathryn{

    struct Rob: Module{

        D_ALL d;
        Table entries;
        mVal(fin, 1,1);
        Slot finSlot;

        mReg(comPtr, RRF_SEL);
        mWire(comPtr2, RRF_SEL);

        mWire(com1En, 1);
        mWire(com2En, 1);


        Rob(D_ALL& din): d(din),
        entries("robTable", robEM, RRF_SEL),
        finSlot({"finish"}, {&fin}){}

        void flow(){

            comPtr2 = comPtr + 1;
            com1En = (cirPtrDis(comPtr, d.rob.dpPtr) >= 1) || d.dis.isRrfFull();
            com2En = (cirPtrDis(comPtr, d.rob.dpPtr) >= 2) || d.dis.isRrfFull();
            auto& cmd1 = d.rob.bcCommit1 = entries.buildGetLogic(comPtr);
            auto& cmd2 = d.rob.bcCommit2 = entries.buildGetLogic(comPtr2);
            d.rob.commit1 = cmd1.at("finish") & com1En;
            d.rob.commit2 = cmd2.at("finish") & com2En;

            cwhile(true)
            offer(ROB){
                ofcc(EX_BR, d.exb.misPred){}
                ofdef{
                    comPtr <<= comPtr + d.rob.commit1 + d.rob.commit2;
                    ////////// to do signal all that rely on commit OFFER


                }
            }

        }

        void alloc(Operable& rrfId, const Slot& dp){ /// just alloc
            entries.assign(dp, rrfId, true);
        }

        void setfin(Operable& rrfId, const Slot& exec){
            entries.assign(exec + finSlot, rrfId, true);
        }



    };

}

#endif //KATRHYN_SRC_KRIDE_ROB_H
