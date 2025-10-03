//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSV_H
#define KATHRYN_SRC_EXAMPLE_O3_RSV_H


#include "kathryn.h"
#include "slotParam.h"
#include "stageStruct.h"

namespace kathryn::o3{

    struct RsvBase{
        SlotMeta _meta;
        Table    _table;

        RsvBase(SlotMeta meta, int amtRow):
            _meta(meta), _table(meta, amtRow){
            _table.doReset();
        }

        virtual ~RsvBase() = default;

        virtual pair<Operable&, OH> buildFreeIndex(OH* exceptIdx){assert(false);}
        virtual RegSlot buildIssue(SyncMeta& syncMeta, BroadCast& bc) = 0;

        Operable& slotReady(WireSlot& iw){
            return iw(busy) && iw(rsValid_1) && iw(rsValid_2);
        }

        void tryOwSpecBit(RegSlot& resultRegSlot, WireSlot& iw, BroadCast& bc){
            ///////// we have to override the spec bit if it is on the fly
            auto& isSpec    = iw(spec);
            auto& specTagIdx= iw(specTag);
            //// send data

            zif ( isSpec && bc.checkIsSuc(specTagIdx)){
                resultRegSlot(spec) <<= 0;
            }

        }

        virtual void writeEntry(OH ohIdx, WireSlot& iw){
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
            _table[ohIdx] <<= iw;
            SET_ASM_PRI_TO_AUTO();
        }

        virtual void onIssue(opr& issueIdx){
            _table[issueIdx](busy) <<= 0;
        }

        virtual void onMisPred(opr& fixTag){

            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+2);
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
                zif (isBusy & isSpec & mySpecTag){
                    isSpec <<= 0;
                }
            });
        }
        //// update the src register
        virtual void onWriteBack(ByPass bp){
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                for (int i = 1; i <= 2; i++){
                    auto& isBusy     = lhs(busy);
                    auto& useSig     = lhs(str(rsUse_) + toS(i));
                    auto& phyIdx     = lhs(str(phyIdx_) + toS(i));
                    //////// do bypass the system
                    zif (isBusy){
                        zif (useSig & phyIdx(0, 5) == bp.rrfIdx){
                            phyIdx <<= bp.val;
                        }
                    }
                }
            });
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSV_H
