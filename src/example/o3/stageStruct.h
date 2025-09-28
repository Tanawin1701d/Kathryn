//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H

#include "kathryn.h"
#include "mpft.h"
#include "parameter.h"
#include "slotParam.h"
#include "tagGen.h"

namespace kathryn::o3{



    struct FetchStage{
        SlotMeta meta{smFetch};
        RegSlot  raw {smFetch};
        SyncMeta fetchSync;
    };

    struct DecodeStage{
        SlotMeta sharedMeta  {smDecShard};
        SlotMeta decodedMeta {smDecBase };

        RegSlot   dcd1     {decodedMeta};
        RegSlot   dcd2     {decodedMeta};
        WireSlot  dcw1     {decodedMeta};
        WireSlot  dcw2     {decodedMeta};
        RegSlot  dcdShared{sharedMeta};

        SyncMeta decodeSync {"decodeSync"};
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

    struct TagMgmt{
        Mpft   mpft;
        TagGen tagGen;
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
