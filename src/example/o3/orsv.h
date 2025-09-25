//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ORSV_H
#define KATHRYN_SRC_EXAMPLE_O3_ORSV_H

#include "rsv.h"

namespace kathryn::o3{

        struct ORsv: RsvBase{

        ORsv(SlotMeta meta, int amtRow):
            RsvBase(smRsvOI + meta, amtRow){}

        /***
         * SLOT maintainance operation
         * do Only In
         */
        void buildSlotLogic(ByPassPool& bypassPool, BroadCast& bc){
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                for (int i = 1; i <= 2; i++){
                    auto& isBusy = lhs(busy);
                    auto& useSig = lhs(str(rsUse_) + toS(i));
                    auto& phyIdx = lhs(str(phyIdx_) + toS(i));
                    auto& isSpec  = lhs(spec);
                    auto& specTagIdx= lhs(specTag);
                    //////// do bypass the system
                    isBusy.makeResetEvent();
                    zif (isBusy){
                            zif(useSig){
                                bypassPool.tryAssignByPassAll(
                                    phyIdx(0, RRF_SEL),phyIdx);
                            }
                            zif(isSpec){
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
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                lhs(sortBit) <<= 0;
            });
        }

        //// valid index
        pair<Operable&, OH> buildFreeIndex(){
            auto [iw, ohIdx] = _table.doReducOHIdx([&](
             WireSlot& lhs, Operable* lidx,
             WireSlot& rhs, Operable* ridx) -> Operable&{
                return ~lhs(busy); //// we don't care rhs
            });

            return {iw(busy), ohIdx};
        }

        /**
         * ISSUE
         */
        RegSlot buildIssue(SyncMeta& syncMeta, BroadCast& bc) override{

            /*
            * find the free slot
            */
            auto [iw, ohIdx] = _table.doReducOHIdx(
                [&](WireSlot& lhs, Operable* lidx,
                            WireSlot& rhs, Operable* ridx)-> Operable&{

                    auto& busyEq = lhs(busy) == rhs(busy);
                    auto& sortBitEq = lhs(sortBit) == rhs(sortBit);
                    auto& rrftagEq = lhs(rrftag) == rhs(rrftag);

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
                    _table[ohIdx](busy) <<= 0;
                }
            }
            return resultRegSlot;

        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ORSV_H
