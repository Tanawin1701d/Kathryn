//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ORSV_H
#define KATHRYN_SRC_EXAMPLE_O3_ORSV_H

#include "rsv.h"

namespace kathryn::o3{

        struct ORsv: RsvBase{

        mWire(checkIdx, _table.getSufficientIdxSize(true));

        ORsv(SlotMeta meta, int amtRow):
            RsvBase(smRsvO + meta, amtRow){}

        void resetSortBit(){
            SET_ASM_PRI_TO_MANUAL(RSV_SORTBIT_RST_PRED_PRIORITY);
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                lhs(sortBit) <<= 0;
            });
            SET_ASM_PRI_TO_AUTO();
        }

        virtual void writeEntry(OH ohIdx, WireSlot& iw) override{
            resetSortBit();
            RsvBase::writeEntry(ohIdx, iw);
        }

        virtual void writeEntry(opr& binIdx, WireSlot& iw) override{
            resetSortBit();
            RsvBase::writeEntry(binIdx, iw);
        }

        pair<opr&, OH> buildFreeOhIndex(OH* exceptIdx){
            auto [iw, ohIdx] = _table.doReducOHIdx([&](
             WireSlot& lhs, Operable* lidx,
             WireSlot& rhs, Operable* ridx) -> opr&{
                if (exceptIdx == nullptr){
                    return ~lhs(busy); //// we don't care rhs
                }
                return ~lhs(busy) && ((*lidx) != exceptIdx->getIdx());

            });

            return {iw(busy), ohIdx};
        }

        void buildIssue(SyncMeta& syncMeta, BroadCast& bc) override{
            /*
            * find the free slot
            */
            auto [iw, ohIdx] = _table.doReducOHIdx(
                [&](WireSlot& lhs, Operable* lidx,
                    WireSlot& rhs, Operable* ridx)-> Operable&{

                    auto& busyEq    = lhs(busy)    == rhs(busy);
                    auto& sortBitEq = lhs(sortBit) == rhs(sortBit);
                    auto& rrftagEq  = lhs(rrftag)  == rhs(rrftag);

                    return
                        (lhs(busy) && (~rhs(busy))) ||
                        (busyEq && (lhs(sortBit) < rhs(sortBit))) ||
                        (busyEq &&  sortBitEq && (lhs(rrftag) < rhs(rrftag)));
                }
            );

            checkIdx = ohIdx.getIdx();

            /**
             * issue sync
             */

            cwhile(true){
                zyncc(syncMeta, slotReady(iw)){ //// do it with spectag
                    execSrc <<= iw;
                    tryOwSpecBit(iw, bc);
                    //////// reset the table
                    onIssue(ohIdx);
                }
            }
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ORSV_H
