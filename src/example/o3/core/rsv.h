//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSV_H
#define KATHRYN_SRC_EXAMPLE_O3_RSV_H


#include "kathryn.h"
#include "slotParam.h"
#include "stageStruct.h"

namespace kathryn::o3{

    ////////////////////////////////////////////////
    ///  expected priority
    ///  |    g1    |    g2                      |          g3
    ///  |mispredict|writeEntry > update sort bit| sucPred/bypass/issue
    ///  g1, g2, g3 cannot happend at the same time
    ///  ////////////////////////////////////////////////

    static int RSV_MIS_PRED_PRIORITY         = DEFAULT_UE_PRI_USER + 3;
    static int RSV_WRITE_ENTRY_PRED_PRIORITY = DEFAULT_UE_PRI_USER + 2;
    static int RSV_SORTBIT_RST_PRED_PRIORITY = DEFAULT_UE_PRI_USER + 1;

    struct RsvBase{
        SlotMeta _meta;
        Table    _table;
        RegSlot  execSrc;

        ZyncSimProb*   issueProbe = nullptr;
        TableSimProbe* stationProbe = nullptr;

        void setSimProbe(ZyncSimProb* issueP, TableSimProbe* stationP){
            issueProbe = issueP;
            stationProbe = stationP;
            assert(stationProbe != nullptr);
            stationProbe->init(&_table);
        }

        RsvBase(SlotMeta meta, int amtRow):
            _meta(meta), _table(meta, amtRow),
        execSrc(meta){
            _table.makeColResetEvent(busy, 0);
        }

        virtual ~RsvBase() = default;

        virtual void buildIssue(SyncMeta& syncMeta, BroadCast& bc) = 0;

        Operable& slotReady(WireSlot& iw){
            return iw(busy) && iw(rsValid_1) && iw(rsValid_2);
        }

        void tryOwSpecBit(WireSlot& iw, BroadCast& bc){
            ///////// we have to override the spec bit if it is on the fly
            auto& isSpec    = iw(spec);
            auto& specTagIdx= iw(specTag);
            //// send data

            zif ( isSpec && bc.checkIsSuc(specTagIdx)){
                execSrc(spec) <<= 0;
            }

        }

        virtual void writeEntry(OH ohIdx, WireSlot& iw){
            SET_ASM_PRI_TO_MANUAL(RSV_WRITE_ENTRY_PRED_PRIORITY);
            _table[ohIdx] <<= iw;
            SET_ASM_PRI_TO_AUTO();
        }

        virtual void writeEntry(opr& binIdx, WireSlot& iw){
            SET_ASM_PRI_TO_MANUAL(RSV_WRITE_ENTRY_PRED_PRIORITY);
            _table[binIdx] <<= iw;
            SET_ASM_PRI_TO_AUTO();
        }

        virtual void onIssue(opr& issueIdx){
            _table[issueIdx](busy) <<= 0;
        }
        virtual void onIssue(OH issueOHIdx){
            _table[issueOHIdx](busy) <<= 0;
        }

        virtual void onMisPred(opr& fixTag){

            SET_ASM_PRI_TO_MANUAL(RSV_MIS_PRED_PRIORITY);
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                auto& isBusy    = lhs(busy);
                auto& isSpec    = lhs(spec);
                auto& mySpecTag = lhs(specTag);
                //////// do bypass the system
                zif (isBusy & isSpec & ((mySpecTag&fixTag) != 0)){
                    isBusy <<= 0;
                }
            });
            SET_ASM_PRI_TO_AUTO();


        }
        virtual void onSucPred(opr& sucTag){
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                auto& isBusy  = lhs(busy);
                auto& isSpec  = lhs(spec);
                auto& mySpecTag= lhs(specTag);
                //////// do bypass the system
                zif (isBusy & isSpec & (mySpecTag == sucTag)){
                    isSpec <<= 0;
                }
            });
        }
        //// update the src register
        virtual void onBypass(ByPass bp){
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                for (int i = 1; i <= 2; i++){
                    auto& isBusy     = lhs(busy);
                    auto& isRsValid  = lhs(str(rsValid_) + toS(i));
                    auto& phyIdx     = lhs(str(phyIdx_) + toS(i));
                    //////// do bypass the system
                    zif (isBusy){
                        zif ((~isRsValid) & (phyIdx(0, RRF_SEL) == bp.rrfIdx)){
                            phyIdx <<= bp.val;
                        }
                    }
                }
            });
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSV_H
