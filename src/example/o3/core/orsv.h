//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ORSV_H
#define KATHRYN_SRC_EXAMPLE_O3_ORSV_H

#include "rsv.h"

namespace kathryn::o3{

        struct ORsv: RsvBase{                                 ///MD RSV_SHARED
        const int  RSV_IDX = 0;                               ///PARAM RSV_SHARED
        RegArch&   regArch;                                   ///CTRL_HC+DATA_HC RSV_SHARED
        bool       sortReq = false;                           ///HLH RSV_SHARED
        mWire(checkIdx, _table.getSufficientIdxSize(true));   ///CTRL_HWD RSV_SHARED
        mWire(dbg_isSlotReady, 1); ///DC

        ORsv(int rsv_idx, SlotMeta meta,           ///DATA_HC RSV_SHARED
             int amtRow , RegArch& regArch,        ///CTRL_HC+DATA_HC RSV_SHARED
             const SlotMeta& osm = smRsvO):        ///CTRL_HC RSV_SHARED
            RsvBase(osm + meta, amtRow),           ///CTRL_HC+DATA_HC RSV_SHARED
            RSV_IDX(rsv_idx),                      ///HLH RSV_SHARED
            regArch(regArch),                      ///CTRL_HC+DATA_HC RSV_SHARED
            sortReq(osm.isThereField(sortBit)){}   ///HLH RSV_SHARED

        void resetSortBit(){                                                   ///HLH RSV_SHARED
            SET_ASM_PRI_TO_MANUAL(RSV_SORTBIT_RST_PRED_PRIORITY);              ///CTRL_CL RSV_SHARED
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){                   ///HLH RSV_SHARED
                lhs(sortBit) <<= lhs(sortBit) & (~regArch.rrf.nextRrfCycle);   ///CTRL_CL RSV_SHARED
            });
            SET_ASM_PRI_TO_AUTO();                                             ///CTRL_CL RSV_SHARED
        }


        void tryWriteEntry(opr& targetIdx, opr& binIdx, WireSlot& iw){   ///CTRL_HC+DATA_HC RSV_SHARED
            zif (targetIdx == RSV_IDX){                                  ///CTRL_CL RSV_SHARED
                RsvBase::writeEntry(binIdx, iw);   ///CTRL_HC+DATA_HC RSV_SHARED
            }
        }

            ////// friend table is used incase of two table join the same index
        pair<opr&, opr&> buildFreeIndex(opr* exceptIdx, RsvBase* friendRsv = nullptr){   ///CTRL_HC RSV_SHARED


            Table effTable = _table; //// = will not build new hardware              ///DATA_DT RSV_SHARED
            if (friendRsv != nullptr){                                               ///HLH RSV_SHARED
                effTable = (effTable.joinTableByRowInterleave(friendRsv->_table));   ///DATA_CL RSV_SHARED
            }

            auto [iw, binIdx] = effTable.doReducBinIdx([&](       ///HLH RSV_SHARED
             WireSlot& lhs, Operable* lidx,                       ///CTRL_HC+DATA_HC RSV_SHARED
             WireSlot& rhs, Operable* ridx) -> opr&{              ///CTRL_HC+DATA_HC RSV_SHARED
                if (exceptIdx == nullptr){                        ///HLH RSV_SHARED
                    return ~lhs(busy); //// we don't care rhs     ///CTRL_CL RSV_SHARED
                }
                return ~lhs(busy) && ((*lidx) != (*exceptIdx));   ///CTRL_CL RSV_SHARED
            });
            return {iw(busy), binIdx};                            ///CTRL_HC RSV_SHARED
        }



        void buildIssue(BroadCast& bc) override{      ///CTRL_HC RSV_SHARED
            /*
            * find the free slot
            */
            auto [iw, ohIdx] = _table.doReducOHIdx(                ///HLH RSV_SHARED
                [&](WireSlot& lhs, Operable* lidx,                 ///CTRL_HC+DATA_HC RSV_SHARED
                    WireSlot& rhs, Operable* ridx)-> Operable&{    ///CTRL_HC+DATA_HC RSV_SHARED
                    lhs.tryAddWire(entry_ready, slotReady(lhs));   ///CTRL_CL RSV_SHARED
                    rhs.tryAddWire(entry_ready, slotReady(rhs));   ///CTRL_CL RSV_SHARED

                    if (sortReq){   ///HLH RSV_SHARED
                        auto& readyEq   = lhs(entry_ready) == rhs(entry_ready);   ///CTRL_CL RSV_SHARED
                        auto& sortBitEq = lhs(sortBit) == rhs(sortBit);           ///CTRL_CL RSV_SHARED
                        return   ///CTRL_CL RSV_SHARED
                            (lhs(entry_ready) && (~rhs(entry_ready))) ||  ///CTRL_CL RSV_SHARED
                            (readyEq && (lhs(sortBit) < rhs(sortBit))) || ///CTRL_CL RSV_SHARED
                            (readyEq &&  sortBitEq && (lhs(rrftag) < rhs(rrftag))); ///CTRL_CL RSV_SHARED
                    }
                    ////// no sort request
                    return lhs(entry_ready);   ///CTRL_HC RSV_SHARED
                }
            );

            checkIdx = ohIdx.getIdx();   ///CTRL_DT RSV_SHARED
            /**
             * issue sync
             */
            dbg_isSlotReady = slotReady(iw); ///DC

            if (sortReq){   ///HLH RSV_SHARED
                resetSortBit();   ///CTRL_CL RSV_SHARED
            }

            cwhile(true){   ///CTRL_HWD+CTRL_CL RSV_SHARED
                zyncc(sync, dbg_isSlotReady){ tryInitProbe(issueProbe);   ///CTRL_HWD+CTRL_CL RSV_SHARED
                    //////// reset the table
                    onIssue(ohIdx, iw);     ///CTRL_HC+DATA_HC RSV_SHARED
                    tryOwSpecBit(iw, bc);   ///CTRL_HC+DATA_HC RSV_SHARED
                }
            }
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ORSV_H
