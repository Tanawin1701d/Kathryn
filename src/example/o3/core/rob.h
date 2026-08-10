//
// Created by tanawin on 30/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ROB_H
#define KATHRYN_SRC_EXAMPLE_O3_ROB_H

#include "stageStruct.h"

namespace kathryn::o3{

    struct StoreBuf;   ///HLH ROB

    struct Rob: Module{                                ///MD ROB
        Table _table;                                  ///CTRL_HWD ROB

        mWire(com1Status, 1      );                   ///CTRL_HWD ROB
        mWire(com2Status, 1      );                   ///CTRL_HWD ROB
        mReg (comPtr    , RRF_SEL);                   ///CTRL_HWD ROB
        mWire(comPtr2   , RRF_SEL);                   ///CTRL_HWD ROB
        PipStage&  pm;                                ///CTRL_HC+DATA_HC ROB
        WireSlot   com1Entry{_table[comPtr  ].v()};   ///CTRL_HWD+DATA_HWD ROB
        WireSlot   com2Entry{_table[comPtr+1].v()};   ///CTRL_HWD+DATA_HWD ROB
        WireSlot   selectedEntry{smROB};              ///CTRL_HWD+DATA_HWD ROB
        RegArch&   regArch;                           ///CTRL_HC+DATA_HC ROB
        StoreBuf&  storeBuf;                          ///CTRL_HC+DATA_HC ROB

        Rob(PipStage& pipStage, RegArch& regArch,   ///CTRL_HC+DATA_HC ROB
            StoreBuf& storeBuf):                    ///CTRL_HC+DATA_HC ROB
            _table(smROB, RRF_NUM),                 ///CTRL_HWD ROB
            pm(pipStage),                           ///CTRL_HC+DATA_HC ROB
            regArch(regArch),                       ///CTRL_HC+DATA_HC ROB
            storeBuf(storeBuf){                     ///CTRL_HC+DATA_HC ROB
            _table.makeColResetEvent(wbFin, 0);     ///CTRL_DT ROB
            _table.makeColResetEvent(isBranch, 0);  ///CTRL_DT ROB
            comPtr.makeResetEvent();                ///CTRL_DT ROB
            dataStructProbGrp.commit.init(&_table); ///DC
        }

        opr& getComPtr(){ return comPtr;}   ///CTRL_HC ROB

        WireSlot& getBranchUpdateEntry(){ return selectedEntry;}   ///CTRL_HC ROB

        void flow() override;   ///HLH ROB

        void onDispatch(opr& idx, RegSlot& dpValue, RegSlot& dpShareVal){   ///CTRL_HC+DATA_HC ROB
            opr& opc = dpValue(inst)(0, 7);                        ///DATA_CL ROB
            _table[idx](wbFin) <<= 0;                              ///CTRL_DT ROB
            _table[idx](storeBit) <<= (opc == RV32_STORE);         ///CTRL_CL ROB
            _table[idx] <<= dpValue;  //// sBranch, rdUse, rdIdx   ///DATA_DT ROB
            _table[idx] <<= dpShareVal; ///  bhr, pc               ///DATA_DT ROB
        }

        void onWriteBack(opr& idx){   ///CTRL_HC ROB
            _table[idx](wbFin) <<= 1;   ///CTRL_DT ROB
        }
    };



}
#endif //KATHRYN_SRC_EXAMPLE_O3_ROB_H
