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

        mWire(com1Status, 1      );
        mWire(com2Status, 1      );
        mReg (comPtr    , RRF_SEL);
        mWire(comPtr2   , RRF_SEL);
        PipStage& pm;
        WireSlot com1Entry{_table[comPtr  ].v()};
        WireSlot com2Entry{_table[comPtr+1].v()};
        RegArch& regArch;

        Rob(PipStage& pipStage, RegArch& regArch):
            _table(smROB, RRF_NUM),
            pm(pipStage),
            regArch(regArch){
            _table.makeColResetEvent(wbFin, 0);
            _table.makeColResetEvent(isBranch, 0);
            comPtr.makeResetEvent();
            dataStructProbGrp.commit.init(&_table);
        }

        opr& getComPtr(){ return comPtr;}


        void flow() override{
            comPtr2 = comPtr + 1;
            comPtr <<= (comPtr + com1Status + com2Status);
            ////// we have to set commit commad

            pip(pm.cm.sync){autoSync
                /////// commit the instruction
                ////// due to branch can do only one
                opr& com1Cond = com1Entry(wbFin);
                opr& com2Cond = com2Entry(wbFin) & ~com1Entry(isBranch);
                ////// rrf commit
                std::tie(com1Status, com2Status) =
                    regArch.rrf.onCommit(comPtr, com1Cond, com2Cond);
                ////// arf commit
                regArch.arf.onCommit(
                    //// com1
                    com1Status & com1Entry(rdUse), comPtr ,
                    com1Entry(rdIdx),
                    regArch.rrf.getPhyData(comPtr),
                    //// com2
                    com2Status & com2Entry(rdUse), comPtr2,
                    com2Entry(rdIdx),
                    regArch.rrf.getPhyData(comPtr2)
                );
            }

        }

        void onDispatch(opr& idx, RegSlot& dpValue){
            _table[idx] <<= dpValue;
            _table[idx](wbFin) <<= 0;
        }

        void onWriteBack(opr& idx){
            _table[idx](wbFin) <<= 1;
        }

    };


}

#endif //KATHRYN_SRC_EXAMPLE_O3_ROB_H
