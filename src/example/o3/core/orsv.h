//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ORSV_H
#define KATHRYN_SRC_EXAMPLE_O3_ORSV_H

#include "rsv.h"

namespace kathryn::o3{

        struct ORsv: RsvBase{
        const int  RSV_IDX = 0;
        RegArch&   regArch;
        bool       sortReq = false;
        mWire(checkIdx, _table.getSufficientIdxSize(true));
        mWire(dbg_isSlotReady, 1);

        ORsv(int rsv_idx, SlotMeta meta,
             int amtRow , RegArch& regArch,
             const SlotMeta& osm = smRsvO):
            RsvBase(osm + meta, amtRow),
            RSV_IDX(rsv_idx),
            regArch(regArch),
            sortReq(osm.isThereField(sortBit)){}

        void resetSortBit(){
            SET_ASM_PRI_TO_MANUAL(RSV_SORTBIT_RST_PRED_PRIORITY);
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                lhs(sortBit) <<= lhs(sortBit) & (~regArch.rrf.nextRrfCycle);

            });
            SET_ASM_PRI_TO_AUTO();
        }


        void tryWriteEntry(opr& targetIdx, opr& binIdx, WireSlot& iw){
            zif (targetIdx == RSV_IDX){
                if (sortReq){
                    resetSortBit();
                }
                RsvBase::writeEntry(binIdx, iw);
            }
        }

            ////// friend table is used incase of two table join the same index
        pair<opr&, opr&> buildFreeIndex(opr* exceptIdx, RsvBase* friendRsv = nullptr){


            Table effTable = _table; //// = will not build new hardware
            if (friendRsv != nullptr){
                effTable = (effTable.joinTableByRowInterleave(friendRsv->_table));
            }

            auto [iw, binIdx] = effTable.doReducBinIdx([&](
             WireSlot& lhs, Operable* lidx,
             WireSlot& rhs, Operable* ridx) -> opr&{
                if (exceptIdx == nullptr){
                    return ~lhs(busy); //// we don't care rhs
                }
                return ~lhs(busy) && ((*lidx) != (*exceptIdx));
            });
            return {iw(busy), binIdx};
        }

        

        void buildIssue(SyncMeta& syncMeta, BroadCast& bc) override{
            /*
            * find the free slot
            */
            auto [iw, ohIdx] = _table.doReducOHIdx(
                [&](WireSlot& lhs, Operable* lidx,
                    WireSlot& rhs, Operable* ridx)-> Operable&{
                    lhs.tryAddWire(entry_ready, slotReady(lhs));
                    rhs.tryAddWire(entry_ready, slotReady(rhs));

                    if (sortReq){
                        auto& readyEq   = lhs(entry_ready) == rhs(entry_ready);
                        auto& sortBitEq = lhs(sortBit) == rhs(sortBit);
                        return
                            (lhs(entry_ready) && (~rhs(entry_ready))) ||
                            (readyEq && (lhs(sortBit) < rhs(sortBit))) ||
                            (readyEq &&  sortBitEq && (lhs(rrftag) < rhs(rrftag)));
                    }
                    ////// no sort request
                    return lhs(entry_ready);
                }
            );

            checkIdx = ohIdx.getIdx();
            /**
             * issue sync
             */
            dbg_isSlotReady = slotReady(iw);

            cwhile(true){
                zyncc(syncMeta, dbg_isSlotReady){ tryInitProbe(issueProbe);
                    tryOwSpecBit(iw, bc);
                    //////// reset the table
                    onIssue(ohIdx, iw);
                }
            }
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ORSV_H
