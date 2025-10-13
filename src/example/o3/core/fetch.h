//
// Created by tanawin on 5/10/25.
//

#ifndef KATHRYN_FETCH_H
#define KATHRYN_FETCH_H

#include "kathryn.h"
#include "stageStruct.h"
#include "parameter.h"

namespace kathryn::o3{

    struct FetchMod : Module{
        PipStage& pm;
        Reg&      curPc;

        mMem (iMem0, IMEM_ROW, IMEM_WIDTH);
        mMem (iMem1, IMEM_ROW, IMEM_WIDTH);
        mMem (iMem2, IMEM_ROW, IMEM_WIDTH);
        mMem (iMem3, IMEM_ROW, IMEM_WIDTH);

        explicit FetchMod(PipStage&  pm) :
        pm(pm),
        curPc(pm.ft.curPc){
            curPc.makeResetEvent();
        }

        void flow() override{

            pip(pm.ft.sync){ autoStart
                zync(pm.dc.sync){
                    selLog();
                }
            }

        }

        void onMisPred(opr& RenewPc){
            curPc <<= RenewPc;
        }

        void selLog(){
            opr& selIdx = curPc(2, 4);
            RegSlot& raw = pm.ft.raw;

            raw(invalid1) <<= 0;
            raw(invalid2) <<= 0;
            raw(pc)       <<= curPc;
            raw(npc)      <<= curPc + 8;
            raw(prCond)   <<= 0;
            raw(bhr)      <<= 0;

            curPc         <<= curPc + 8;

            auto& i0 = iMem0[curPc(4, INSN_LEN)];
            auto& i1 = iMem1[curPc(4, INSN_LEN)];
            auto& i2 = iMem2[curPc(4, INSN_LEN)];
            auto& i3 = iMem3[curPc(4, INSN_LEN)];

            zif(selIdx == 0){
                raw(inst1)    <<=  i0;
                raw(inst2)    <<=  i1;
            }zelif(selIdx == 1){
                raw(inst1)    <<=  i1;
                raw(inst2)    <<=  i2;
            }zelif(selIdx == 2){
                raw(inst1)    <<=  i2;
                raw(inst2)    <<=  i3;
            }zelse{
                raw(inst1)    <<= i3;
                raw(inst2)    <<= i0;
                raw(invalid2) <<= 1;
                raw(npc)      <<= (curPc + 4);
                curPc         <<= curPc + 4;
            }
        }
    };

}

#endif //KATHRYN_FETCH_H