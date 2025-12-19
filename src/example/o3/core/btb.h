//
// Created by tanawin on 13/12/25.
//

#ifndef EXAMPLE_O3_CORE_BTB_H
#define EXAMPLE_O3_CORE_BTB_H

#include "parameter.h"
#include "slotParam.h"

namespace kathryn::o3{

    struct Btb{
        Table validTable{ smBtb, 8}; /// 8*64 = 512    /// todo change tot
        //mMem(valid, BTB_IDX_NUM, 1); ///// it is supposed to be register
        mMem(bia  , BTB_IDX_NUM, ADDR_LEN);
        mMem(bta  , BTB_IDX_NUM, ADDR_LEN);

        Btb(){
            validTable.makeResetEvent(0, CM_NEGEDGE);
        }

        void onCommit(opr& srcAddr, opr& targetAddr){
            opr& btbAddr  = srcAddr.sl(BTB_IDX_SEL_START, BTB_IDX_SEL_STOP);
            opr& btbAddrL = btbAddr.sl(0, 6);
            opr& btbAddrH = btbAddr.sl(6, 9);
            mVal(one, 64, 1);
            opr& commitedBit = one << btbAddrL;
            SET_CLK_MODE2NEG_EDGE();
                validTable.doCusLogic([&](RegSlot& regSlot, int rowIdx){
                    zif ( btbAddrH == rowIdx){
                        regSlot(0) <<= ( regSlot(0) | commitedBit );
                    }
                });
                bia  [btbAddr] <<= srcAddr;
                bta  [btbAddr] <<= targetAddr;
            SET_CLK_MODE2DEF();
        }

        ///// hit and address
        std::pair<opr&, opr&> onInquire(opr& cur_pc, opr& inv2){
            /////// pre declaration
            opr& btbAddr  = cur_pc.sl(BTB_IDX_SEL_START, BTB_IDX_SEL_STOP);
            opr& btbAddrL = btbAddr.sl(0, 6);
            opr& btbAddrH = btbAddr.sl(6, 9);
            opr& rowValid = (validTable[btbAddrH](0).v() >> btbAddrL).sl(0);
            /////// retrieve the data from the memory
            mReg(tagData   , ADDR_LEN);
            mReg(targetAddr, ADDR_LEN);
            SET_CLK_MODE2NEG_EDGE();
            tagData    <<= bia[btbAddr];
            targetAddr <<= bta[btbAddr];
            SET_CLK_MODE2DEF();
            /////// check it is hit or not
            opr& hit1 = (tagData == cur_pc) && rowValid;
            opr& hit2 = (tagData == cur_pc) && rowValid &&
                        (~inv2);

            return {hit1 | hit2, targetAddr};
        }
    };

}

#endif //KATHRYN_BTB_H