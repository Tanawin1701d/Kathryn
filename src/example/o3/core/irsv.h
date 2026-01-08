//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IRSV_H
#define KATHRYN_SRC_EXAMPLE_O3_IRSV_H

#include "rsv.h"
#include "search_idx.h"


namespace kathryn::o3{



    struct IRsv: RsvBase{

        mWire(checkIdx, _table.getSufficientIdxSize(false));
        const int  RSV_IDX = 0;
        Reg& allocPtr;
        SearchResult b1 ,  e1,  e0;
        SearchResult nb1, ne1, nb0; /// search to fix alloc pointer

        mWire(dbg_b1_valid, 1);
        mWire(dbg_e1_valid, 1);
        mWire(dbg_e0_valid, 1);
        mWire(dbg_nb1_valid, 1);
        mWire(dbg_ne1_valid, 1);
        mWire(dbg_nb0_valid, 1);

        mWire(dbg_b1_idx, 3);
        mWire(dbg_e1_idx, 3);
        mWire(dbg_e0_idx, 3);
        mWire(dbg_nb1_idx, 3);
        mWire(dbg_ne1_idx, 3);
        mWire(dbg_nb0_idx, 3);

        IRsv(int rsv_idx  , SlotMeta meta,
             int indexSize, std::string debugName,
             BroadCast& bc):
            RsvBase(smRsvI + meta, 1 << indexSize),
            RSV_IDX(rsv_idx),
            allocPtr(mOprReg("allocPtr_" + debugName, indexSize)),
            b1 (searchIdx(_table, 1, true , bc, false)),
            e1 (searchIdx(_table, 1, false, bc, false)),
            e0 (searchIdx(_table, 0, false, bc, false)),
            nb1(searchIdx(_table, 1, true , bc, true )),
            ne1(searchIdx(_table, 1, false, bc, true )),
            nb0(searchIdx(_table, 0, true , bc, true )){
            allocPtr.makeResetEvent();
        }


        void tryWriteEntry(opr& targetIdx, opr& binIdx, WireSlot& iw){
            zif (targetIdx == RSV_IDX){
                allocPtr <<= (binIdx + 1);
                RsvBase::writeEntry(binIdx, iw);
            }
        }

        void onMisPred(opr& fixTag) override{

            RsvBase::onMisPred(fixTag);
            zif (nb0.sValid){ /// there is empty space for next update
                /// if there is no 1 for next cycle  (the )
                zif(~nb1.sValid){
                    allocPtr <<= 1; ////// it is empty
                }zelif((nb1.sIdx == 0) && (ne1.sIdx == (_table.getNumRow()-1))){
                    ////// case 0  is bubble there is 1 atstart and 1 at the end
                    allocPtr <<= nb0.sIdx; ///// nb0
                }zelse{
                    allocPtr <<= (ne1.sIdx+1);
                }
            }
            /// incase update when entrance the filler at rename stage will handle it
        }

        /**
         * ISSUE
         */

        pair<opr&, opr&> buildFreeIndex(opr* reqIdx, RsvBase* friendRsv = nullptr){
            assert(friendRsv == nullptr);

            opr* selIdx = (reqIdx == nullptr)? &allocPtr : reqIdx;

            return {_table[*selIdx](busy).v(), *selIdx};

        }

        void buildIssue(SyncMeta& syncMeta, BroadCast& bc) override{

            /*
             *  the required Idx
             */
            zif(e0.sValid){ ///  there is empty slot
                zif((b1.sIdx == 0) && (e1.sIdx == (_table.getNumRow()-1))){ ///// zero bubble
                    //// it may be entirely zeros but we will check it at slotReady(iw)
                    checkIdx = (e0.sIdx + 1);
                }zelse{ checkIdx = b1.sIdx; }
            }zelse{
                checkIdx = allocPtr;
            }

            /**
             * the issue block
             */
            WireSlot iw(_table[checkIdx].v());

            cwhile(true){
                zyncc(syncMeta, slotReady(iw)){ tryInitProbe(issueProbe);
                    //////// reset the table
                    onIssue(checkIdx, iw); //// reset busy
                    tryOwSpecBit(iw, bc);
                }
            }

            dbg_b1_valid     = b1.first;
            dbg_e1_valid     = e1.first;
            dbg_e0_valid     = e0.first;
            dbg_nb1_valid     = nb1.first;
            dbg_ne1_valid     = ne1.first;
            dbg_nb0_valid     = nb0.first;

            dbg_b1_idx    =    b1.second;
            dbg_e1_idx    =    e1.second;
            dbg_e0_idx    =    e0.second;
            dbg_nb1_idx    =    nb1.second;
            dbg_ne1_idx    =    ne1.second;
            dbg_nb0_idx    =    nb0.second;
        }

    };


}

#endif //KATHRYN_SRC_EXAMPLE_O3_IRSV_H
