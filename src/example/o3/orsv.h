//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ORSV_H
#define KATHRYN_SRC_EXAMPLE_O3_ORSV_H

#include "rsv.h"

namespace kathryn::o3{

        struct ORsv: RsvBase{

        ORsv(SlotMeta meta, int amtRow):
            RsvBase(smRsvO + meta, amtRow){}

        /***
         * SLOT maintainance operation
         * do Only In
         */
        void buildSlotLogic(ByPassPool& bypassPool, BroadCast& bc){
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                for (int i = 1; i <= 2; i++){
                    auto& isBusy  = lhs(busy);
                    auto& useSig  = lhs(str(rsUse_) + toS(i));
                    auto& phyIdx  = lhs(str(phyIdx_) + toS(i));
                    auto& isSpec  = lhs(spec);
                    auto& specTagIdx= lhs(specTag);
                    //////// do bypass the system
                    isBusy.makeResetEvent();
                    zif (isBusy){
                            zif(useSig){
                                bypassPool.tryAssignByPassAll(
                                    phyIdx(0, RRF_SEL),phyIdx);
                            }
                            zif(isSpec){ ////// kill checking
                                zif(bc.checkIsKill(specTagIdx)){
                                    isBusy <<= 0; //// kill the system
                                }
                                zif(bc.checkIsSuc(specTagIdx)){
                                    isSpec <<= 0; ///// verified
                                }
                            }
                    }
                    //////// do kill the slot i
                }
            });
        }

        void resetSortBit(){
            SET_ASM_PRI_TO_MANUAL(RSV_SORTBIT_RST_PRED_PRIORITY);
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                lhs(sortBit) <<= 0;
            });
            SET_ASM_PRI_TO_AUTO();
        }

        virtual void writeEntry(OH ohIdx, WireSlot& iw){
            resetSortBit();
            RsvBase::writeEntry(ohIdx, iw);
        }

        virtual void writeEntry(opr& binIdx, WireSlot& iw){
            resetSortBit();
            RsvBase::writeEntry(binIdx, iw);
        }

        pair<Operable&, OH> buildFreeIndex(OH* exceptIdx) override{
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

        RegSlot buildIssue(SyncMeta& syncMeta, BroadCast& bc) override{
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

            /**
             * issue sync
             */

            RegSlot resultRegSlot(_meta);
            cwhile(true){
                zyncc(syncMeta, slotReady(iw)){ //// do it with spectag
                    resultRegSlot <<= iw;
                    tryOwSpecBit(resultRegSlot, iw, bc);
                    //////// reset the table
                    onIssue(ohIdx);
                }
            }
            return resultRegSlot;
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ORSV_H
