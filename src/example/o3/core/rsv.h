//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSV_H
#define KATHRYN_SRC_EXAMPLE_O3_RSV_H

#include "stageStruct.h"

namespace kathryn::o3{

    ////////////////////////////////////////////////
    ///  expected priority
    ///  |    g1    |    g2                      |          g3
    ///  |mispredict|writeEntry > update sort bit| sucPred/bypass/issue
    ///  g1, g2, g3 cannot happend at the same time
    ///  ////////////////////////////////////////////////

    static int RSV_MIS_PRED_PRIORITY         = DEFAULT_UE_PRI_USER + 3;   ///PARAM RSV_SHARED
    static int RSV_WRITE_ENTRY_PRED_PRIORITY = DEFAULT_UE_PRI_USER + 2;   ///PARAM RSV_SHARED
    static int RSV_SORTBIT_RST_PRED_PRIORITY = DEFAULT_UE_PRI_USER + 1;   ///PARAM RSV_SHARED

    struct RsvBase{                        ///MD RSV_SHARED
        SlotMeta _meta;                    ///DATA_HWD RSV_SHARED
        Table    _table;                   ///CTRL_HWD+DATA_HWD RSV_SHARED
        RegSlot  execSrc;                  ///CTRL_HWD+DATA_HWD RSV_SHARED
        SyncPip  sync {"rsv_exec_sync"};   ///MD RSV_SHARED

        ZyncSimProb*   issueProbe = nullptr;     ///DC
        TableSimProbe* stationProbe = nullptr;   ///DC

        void setSimProbe(ZyncSimProb* issueP, TableSimProbe* stationP){  ///DC
            issueProbe = issueP;                                         ///DC
            stationProbe = stationP;                                     ///DC
            assert(stationProbe != nullptr);                             ///DC
            stationProbe->init(&_table);                                 ///DC
        }                                                                ///DC

        RsvBase(const SlotMeta& meta, int amtRow):   ///CTRL_HC+DATA_HC RSV_SHARED
        _meta(meta),_table(meta, amtRow),            ///CTRL_HWD+DATA_HWD RSV_SHARED
        execSrc(meta){                               ///CTRL_HWD+DATA_HWD RSV_SHARED
            //_table.makeColResetEvent(busy, 0);
            _table.makeResetEvent(0);                ///CTRL_DT+DATA_DT RSV_SHARED
            execSrc.makeResetEvent();                ///CTRL_DT+DATA_DT RSV_SHARED
        }

        virtual ~RsvBase() = default;   ///HLH RSV_SHARED

        virtual void buildIssue(BroadCast& bc) = 0;   ///CTRL_HC RSV_SHARED

        Operable& slotReady(WireSlot& iw){   ///CTRL_HC+DATA_HC RSV_SHARED
            return iw(busy) && iw(rsValid_1) && iw(rsValid_2);   ///CTRL_CL RSV_SHARED
        }

        void tryOwSpecBit(WireSlot& iw, BroadCast& bc){   ///CTRL_HC+DATA_HC RSV_SHARED
            ///////// we have to override the spec bit if it is on the fly
            // auto& isSpec    = iw(spec);
            // auto& specTagIdx= iw(specTag);
            // //// send data
            //
            // zif ( isSpec && bc.checkIsSuc(specTagIdx)){
            //     execSrc(spec) <<= 0;
            // }
            zif ( bc.checkIsSuc(iw)){   ///CTRL_CL RSV_SHARED
                execSrc(spec) <<= 0;    ///CTRL_DT RSV_SHARED
            }
        }

        virtual void writeEntry(opr& binIdx, WireSlot& iw){         ///CTRL_HC+DATA_HC RSV_SHARED
            SET_ASM_PRI_TO_MANUAL(RSV_WRITE_ENTRY_PRED_PRIORITY);   ///CTRL_CL RSV_SHARED
            _table[binIdx] <<= iw;                                  ///DATA_DT RSV_SHARED
            SET_ASM_PRI_TO_AUTO();                                  ///CTRL_CL RSV_SHARED
        }

        virtual void onIssue(opr& issueIdx, WireSlot& iw){   ///CTRL_HC+DATA_HC RSV_SHARED
            execSrc <<= iw;                                  ///CTRL_DT+DATA_DT RSV_SHARED
            _table[issueIdx](busy) <<= 0;                    ///CTRL_DT RSV_SHARED
        }
        virtual void onIssue(OH issueOHIdx, WireSlot& iw){   ///CTRL_HC+DATA_HC RSV_SHARED
            execSrc <<= iw;                                  ///CTRL_DT+DATA_DT RSV_SHARED
            _table[issueOHIdx](busy) <<= 0;                  ///CTRL_DT RSV_SHARED
        }

        virtual void onMisPred(opr& fixTag){  ///CTRL_HC RSV_SHARED

            SET_ASM_PRI_TO_MANUAL(RSV_MIS_PRED_PRIORITY);            ///CTRL_CL RSV_SHARED
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){         ///HLH RSV_SHARED
                auto& isBusy    = lhs(busy);                         ///CTRL_DT RSV_SHARED
                auto& isSpec    = lhs(spec);                         ///CTRL_DT RSV_SHARED
                auto& mySpecTag = lhs(specTag);                      ///CTRL_DT RSV_SHARED
                //////// do bypass the system
                zif (isBusy & isSpec & ((mySpecTag&fixTag) != 0)){   ///CTRL_CL RSV_SHARED
                    isBusy <<= 0;                                    ///CTRL_DT RSV_SHARED
                }
            });
            SET_ASM_PRI_TO_AUTO();                                   ///CTRL_CL RSV_SHARED


        }
        virtual void onSucPred(opr& sucTag){                   ///CTRL_HC RSV_SHARED
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){   ///HLH RSV_SHARED
                auto& isBusy  = lhs(busy);                     ///CTRL_DT RSV_SHARED
                auto& isSpec  = lhs(spec);                     ///CTRL_DT RSV_SHARED
                auto& mySpecTag= lhs(specTag);                 ///CTRL_DT RSV_SHARED
                //////// do bypass the system
                zif (isBusy & isSpec & (mySpecTag == sucTag)){ ///CTRL_CL RSV_SHARED
                    isSpec <<= 0;                              ///CTRL_DT RSV_SHARED
                }
            });
        }
        //// update the src register
        virtual void onBypass(ByPass bp){                                         ///CTRL_HC+DATA_HC RSV_SHARED
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){                      ///HLH RSV_SHARED
                for (int i = 1; i <= 2; i++){                                     ///HLH RSV_SHARED
                    auto& isBusy     = lhs(busy);                                 ///CTRL_DT RSV_SHARED
                    auto& isRsValid  = lhs(str(rsValid_) + toS(i));               ///CTRL_DT RSV_SHARED
                    auto& phyIdx     = lhs(str(phyIdx_) + toS(i));                ///DATA_DT RSV_SHARED
                    //////// do bypass the system
                    zif (isBusy){                                                 ///CTRL_CL RSV_SHARED
                        zif ((~isRsValid) & (phyIdx(0, RRF_SEL) == bp.rrfIdx)){   ///CTRL_CL RSV_SHARED
                            phyIdx <<= bp.val;                                    ///DATA_DT RSV_SHARED
                            isRsValid <<= 1;                                      ///CTRL_DT RSV_SHARED
                        }
                    }
                }
            });
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSV_H
