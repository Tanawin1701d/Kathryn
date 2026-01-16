//
// Created by tanawin on 30/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ROB_H
#define KATHRYN_SRC_EXAMPLE_O3_ROB_H

#include "stageStruct.h"

namespace kathryn::o3{

    struct StoreBuf;

    struct Rob: Module{
        Table _table;

        mWire(com1Status, 1      );
        mWire(com2Status, 1      );
        mReg (comPtr    , RRF_SEL);
        mWire(comPtr2   , RRF_SEL);
        PipStage&  pm;
        WireSlot   com1Entry{_table[comPtr  ].v()};
        WireSlot   com2Entry{_table[comPtr+1].v()};
        WireSlot   selectedEntry{smROB};
        RegArch&   regArch;
        StoreBuf&  storeBuf;

        Rob(PipStage& pipStage, RegArch& regArch,
            StoreBuf& storeBuf):
            _table(smROB, RRF_NUM),
            pm(pipStage),
            regArch(regArch),
            storeBuf(storeBuf){
            _table.makeColResetEvent(wbFin, 0);
            _table.makeColResetEvent(isBranch, 0);
            comPtr.makeResetEvent();
            dataStructProbGrp.commit.init(&_table);
        }

        opr& getComPtr(){ return comPtr;}

        WireSlot& getBranchUpdateEntry(){ return selectedEntry;}

        void flow() override;

        void onDispatch(opr& idx, RegSlot& dpValue, RegSlot& dpShareVal){
            opr& opc = dpValue(inst)(0, 7);
            _table[idx](wbFin) <<= 0;
            _table[idx](storeBit) <<= (opc == RV32_STORE);
            _table[idx] <<= dpValue;  //// sBranch, rdUse, rdIdx
            _table[idx] <<= dpShareVal; ///  bhr, pc
        }

        void onWriteBack(opr& idx){
            _table[idx](wbFin) <<= 1;
        }
    };



}
#endif //KATHRYN_SRC_EXAMPLE_O3_ROB_H
