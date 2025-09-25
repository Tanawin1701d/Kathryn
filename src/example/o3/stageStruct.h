//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H

#include "kathryn.h"
#include "parameter.h"
#include "slotParam.h"

namespace kathryn::o3{



    struct FetchStage{
        SlotMeta meta {smFetch};
        RegSlot  dayta{smFetch};
    };

    struct DecodeStage{
        SlotMeta sharedMeta  {smDecShard};
        SlotMeta decodedMeta {smDecBase +
            buildArchRegSlotMeta(1, SRC_A_SEL_WIDTH) +
            buildArchRegSlotMeta(2, SRC_B_SEL_WIDTH)
        };

        RegSlot  ins1     {decodedMeta};
        RegSlot  ins2     {decodedMeta};
        RegSlot  sharddata{sharedMeta };
    };

    struct ByPassPool{

        std::vector<Operable*> _byPassRrfIdx;
        std::vector<Operable*> _byPassVal;
        std::vector<Operable*> _activeSignal;

        void addByPassValue(Operable* byPassRrfIdx, Operable* byPassVal, Operable* activeSignal){
            if (!byPassRrfIdx || !byPassVal || !activeSignal){
                throw std::invalid_argument("Null pointer passed to addByPassValue");
            }
            _byPassRrfIdx.push_back(byPassRrfIdx);
            _byPassVal   .push_back(byPassVal   );
            _activeSignal.push_back(activeSignal);
        }

        void tryAssignByPass(int idx, Operable& desIdent, Reg& desVal){
            zif( (desIdent == *_byPassRrfIdx[idx]) && *_activeSignal[idx]){
                desVal <<= *_byPassVal[idx];
            }
        }

        void tryAssignByPassAll(Operable& desIdent, Reg& desVal){
            for (int idx = 0; idx < _byPassRrfIdx.size(); idx++){
                tryAssignByPass(idx, desIdent, desVal);
            }
        }

    };

    struct BroadCast{
        mWire(miss, 1);
        mWire(succ, 1);
        Operable& isBrMissPred(){ return miss;}
        Operable& isBrSuccPred(){ return succ;}
        Operable& checkIsKill(Operable& specIdx){}
        Operable& checkIsSuc(Operable& specIdx){}

    };




}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
