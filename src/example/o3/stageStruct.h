//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H


#include "kathryn.h"

#include "parameter.h"
#include "slotParam.h"

#include "tagGen.h"
#include "mpft.h"

#include "arf.h"
#include "rrf.h"
#include "rob.h"

namespace kathryn::o3{



    struct FetchStage{
        SlotMeta meta{smFetch};
        RegSlot  raw {smFetch};
        SyncMeta sync;
    };

    struct DecodeStage{
        SlotMeta sharedMeta  {smDecShard};
        SlotMeta decodedMeta {smDecBase };

        RegSlot   dcd1    {decodedMeta};
        RegSlot   dcd2    {decodedMeta};
        WireSlot  dcw1    {decodedMeta};
        WireSlot  dcw2    {decodedMeta};
        RegSlot  dcdShared{sharedMeta};

        SyncMeta sync {"decodeSync"};

        Operable& getIsAlocRsv(RegSlot& dcd){ return dcw1(rsIdx_1); }

    };

    struct DispStage{
        SyncMeta sync {"dispSync"};
    };

    struct ExecStage{
        SyncMeta sync {"execSync"};
    };

    struct BranchStage{
        SyncMeta sync {"branchSync"};
    };

    struct ByPass{
        opr& valid,&rrfIdx, &val;

        void tryAssignByPass(Operable& desIdent, Reg& desVal){
            zif(valid && (desIdent == rrfIdx)){
                desVal <<= val;
            }
        }

    };

    struct ByPassPool{

        std::vector<ByPass> _bps;

        void addByPassValue(opr& valid,opr& rrfIdx, opr& val){
            _bps.push_back({valid, rrfIdx, val});
        }
        
        void tryAssignByPassAll(Operable& desIdent, Reg& desVal){
            for (auto& bp : _bps){
                bp.tryAssignByPass(desIdent, desVal);
            }
        }

    };

    struct BroadCast{
        mWire(mis, 1);
        mWire(suc, 1);
        mWire(misTag, SPECTAG_LEN);
        mWire(fixTag, SPECTAG_LEN);
        mWire(sucTag, SPECTAG_LEN);
        opr& isBrMissPred(){ return mis;}
        opr& isBrSuccPred(){ return suc;}
        opr& checkIsKill(opr& specIdx){
            return mis & (misTag == specIdx);
        }
        opr& checkIsSuc (opr& specIdx){
            return suc & (sucTag == specIdx);
        }
        opr& getSMtag(){return fixTag;}

    };

    struct TagMgmt{
        TagGen tagGen;
        Mpft   mpft;

    };

    struct RegArch{
        Arf arf;
        Rrf rrf;
    };

    struct PipStage{
        FetchStage  ft;
        DecodeStage dc;
        DispStage   ds;
        ExecStage   ex;
        BranchStage br;
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
