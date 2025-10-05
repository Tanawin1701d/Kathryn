//
// Created by tanawin on 30/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ROB_H
#define KATHRYN_SRC_EXAMPLE_O3_ROB_H

#include "kathryn.h"
#include "parameter.h"
#include "slotParam.h"
#include "stageStruct.h"

namespace kathryn::o3{


    struct Rob: public Module{
        Table _table;
        mWire(hold, 1);
        mWire(com1Status, 1);
        mWire(com2Status, 1);
        mReg(comPtr, RRF_SEL);
        mWire(comPtr2, RRF_SEL);
        RegArch& regArch;

        Rob(RegArch& regArch):
            _table(smROB, RRF_NUM),
            regArch(regArch){
            _table.doReset();
            comPtr.makeResetEvent();
        }

        opr& getComPtr(){ return comPtr;}

        void onMisPred(){ hold = 1;}

        void flow() override{
            comPtr2 = comPtr + 1;
            ////// we have to set commit commad

            cwhile(true){ holdBlk(hold)
                /////// commit the instruction
                WireSlot com1Entry = _table[comPtr].v();
                WireSlot com2Entry = _table[comPtr+1].v();
                ////// due to branch can do only one
                opr& com1Cond = com1Entry(wbFin);
                opr& com2Cond = com2Entry(wbFin) & ~com1Entry(isBranch);
                ////// rrf commit
                std::tie(com1Status, com2Status) =
                    regArch.rrf.onCommit(comPtr, com1Cond, com2Cond);
                ////// arf commit
                regArch.arf.onCommit(
                com1Status & com1Entry(rdUse), comPtr , com1Entry(rdIdx),
                com2Status & com2Entry(rdUse), comPtr2, com2Entry(rdIdx)
                );
            }

        }

        void onDispatch(opr& idx, WireSlot dpValue){
            _table[idx] <<= dpValue;
            _table[idx](wbFin) <<= 0;
        }

        void onWriteBack(opr& idx){
            _table[idx](wbFin) <<= 1;
        }

    };


}

#endif //KATHRYN_SRC_EXAMPLE_O3_ROB_H
