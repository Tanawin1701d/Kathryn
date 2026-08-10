//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IRSV_H
#define KATHRYN_SRC_EXAMPLE_O3_IRSV_H

#include "rsv.h"
#include "search_idx.h"


namespace kathryn::o3{



    struct IRsv: RsvBase{   ///MD RSV_SHARED

        mWire(checkIdx, _table.getSufficientIdxSize(false));          ///CTRL_HWD RSV_SHARED
        const int  RSV_IDX = 0;                                       ///PARAM RSV_SHARED
        Reg& allocPtr;                                                ///CTRL_HWD RSV_SHARED
        SearchResult b1 ,  e1,  e0;                                   ///CTRL_HWD RSV_SHARED
        SearchResult nb1, ne1, nb0; /// search to fix alloc pointer   ///CTRL_HWD RSV_SHARED

        mWire(dbg_b1_valid, 1);    ///DC
        mWire(dbg_e1_valid, 1);    ///DC
        mWire(dbg_e0_valid, 1);    ///DC
        mWire(dbg_nb1_valid, 1);   ///DC
        mWire(dbg_ne1_valid, 1);   ///DC
        mWire(dbg_nb0_valid, 1);   ///DC

        mWire(dbg_b1_idx, 3);    ///DC
        mWire(dbg_e1_idx, 3);    ///DC
        mWire(dbg_e0_idx, 3);    ///DC
        mWire(dbg_nb1_idx, 3);    ///DC
        mWire(dbg_ne1_idx, 3);    ///DC
        mWire(dbg_nb0_idx, 3);    ///DC

        IRsv(int rsv_idx  , SlotMeta meta,           ///DATA_HC RSV_SHARED
             int indexSize, std::string debugName,   ///HLH     RSV_SHARED
             BroadCast& bc):                         ///CTRL_HC RSV_SHARED
            RsvBase(smRsvI + meta, 1 << indexSize),  ///CTRL_HC+DATA_HC RSV_SHARED
            RSV_IDX(rsv_idx),   ///HLH RSV_SHARED
            allocPtr(mOprReg("allocPtr_" + debugName, indexSize)),   ///CTRL_HWD RSV_SHARED
            b1 (searchIdx(_table, 1, true , bc, false)),   ///CTRL_HC RSV_SHARED
            e1 (searchIdx(_table, 1, false, bc, false)),   ///CTRL_HC RSV_SHARED
            e0 (searchIdx(_table, 0, false, bc, false)),   ///CTRL_HC RSV_SHARED
            nb1(searchIdx(_table, 1, true , bc, true )),   ///CTRL_HC RSV_SHARED
            ne1(searchIdx(_table, 1, false, bc, true )),   ///CTRL_HC RSV_SHARED
            nb0(searchIdx(_table, 0, true , bc, true )){   ///CTRL_HC RSV_SHARED
            allocPtr.makeResetEvent();                     ///CTRL_DT RSV_SHARED
        }


        void tryWriteEntry(opr& targetIdx, opr& binIdx, WireSlot& iw){   ///CTRL_HC+DATA_HC RSV_SHARED
            zif (targetIdx == RSV_IDX){            ///CTRL_CL RSV_SHARED
                allocPtr <<= (binIdx + 1);         ///CTRL_CL RSV_SHARED
                RsvBase::writeEntry(binIdx, iw);   ///CTRL_HC+DATA_HC RSV_SHARED
            }
        }

        void onMisPred(opr& fixTag) override{   ///CTRL_HC RSV_SHARED

            RsvBase::onMisPred(fixTag);                                            ///CTRL_HC RSV_SHARED
            zif (nb0.sValid){ /// there is empty space for next update             ///CTRL_CL RSV_SHARED
                /// if there is no 1 for next cycle  (the )
                zif(~nb1.sValid){                                                  ///CTRL_CL RSV_SHARED
                    allocPtr <<= 1; ////// it is empty                             ///CTRL_DT RSV_SHARED
                }zelif((nb1.sIdx == 0) && (ne1.sIdx == (_table.getNumRow()-1))){   ///CTRL_CL RSV_SHARED
                    ////// case 0  is bubble there is 1 atstart and 1 at the end
                    allocPtr <<= nb0.sIdx; ///// nb0                               ///CTRL_DT RSV_SHARED
                }zelse{
                    allocPtr <<= (ne1.sIdx+1);                                     ///CTRL_CL RSV_SHARED
                }
            }
            /// incase update when entrance the filler at rename stage will handle it
        }

        /**
         * ISSUE
         */

        pair<opr&, opr&> buildFreeIndex(opr* reqIdx, RsvBase* friendRsv = nullptr){   ///CTRL_HC RSV_SHARED
            assert(friendRsv == nullptr); ///DC

            opr* selIdx = (reqIdx == nullptr)? &allocPtr : reqIdx;   ///HLH RSV_SHARED
            return {_table[*selIdx](busy).v(), *selIdx};             ///CTRL_HC RSV_SHARED
        }

        void buildIssue(BroadCast& bc) override{   ///CTRL_HC RSV_SHARED

            /*
             *  the required Idx
             */
            checkIdx = allocPtr;                                                                ///CTRL_DT RSV_SHARED
            zif(e0.sValid){ ///  there is empty slot                                            ///CTRL_CL RSV_SHARED
                checkIdx = b1.sIdx;                                                             ///CTRL_DT RSV_SHARED
                zif((b1.sIdx == 0) && (e1.sIdx == (_table.getNumRow()-1))){ ///// zero bubble   ///CTRL_CL RSV_SHARED
                    //// it may be entirely zeros but we will check it at slotReady(iw)
                    checkIdx = (e0.sIdx + 1);                                                   ///CTRL_CL RSV_SHARED
                }
            }

            /**
             * the issue block
             */
            WireSlot iw(_table[checkIdx].v());   ///CTRL_DT+DATA_DT RSV_SHARED

            cwhile(true){                        ///CTRL_HWD+CTRL_CL RSV_SHARED
                zyncc(sync, slotReady(iw)){ tryInitProbe(issueProbe);   ///CTRL_HWD+CTRL_CL RSV_SHARED
                    //////// reset the table
                    onIssue(checkIdx, iw); //// reset busy   ///CTRL_HC+DATA_HC RSV_SHARED
                    tryOwSpecBit(iw, bc);   ///CTRL_HC+DATA_HC RSV_SHARED
                }
            }

            dbg_b1_valid     = b1.first;   ///DC
            dbg_e1_valid     = e1.first;   ///DC
            dbg_e0_valid     = e0.first;   ///DC
            dbg_nb1_valid     = nb1.first; ///DC
            dbg_ne1_valid     = ne1.first; ///DC
            dbg_nb0_valid     = nb0.first; ///DC

            dbg_b1_idx    =    b1.second;    ///DC
            dbg_e1_idx    =    e1.second;    ///DC
            dbg_e0_idx    =    e0.second;    ///DC
            dbg_nb1_idx    =    nb1.second;  ///DC
            dbg_ne1_idx    =    ne1.second;  ///DC
            dbg_nb0_idx    =    nb0.second;  ///DC
        }

    };


}

#endif //KATHRYN_SRC_EXAMPLE_O3_IRSV_H
