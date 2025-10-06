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
#include "broadCast.h"
#include "rrf.h"
#include "syncMetaPip.h"

namespace kathryn::o3{



    struct FetchStage{
        mReg(curPc, ADDR_LEN);
        SlotMeta meta{smFetch};
        RegSlot  raw {smFetch};
        SyncMeta sync{"fetchSync"};
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

    struct RsvStage{
        SyncMeta sync {"rsvSync"};
    };

    struct ExecStage{
        SyncPip sync {"execSync"};
    };

    struct BranchStage{
        SyncPip sync {"branchSync"};
    };

    struct ByPass{
        int bpIdx = -1;
        mWire(valid, 1);
        mWire(rrfIdx, RRF_SEL);
        mWire(val, DATA_LEN);

        ByPass(int bpIdx):bpIdx(bpIdx){}

        void tryAssignByPass(Operable& desIdent, Reg& desVal){
            zif(valid && (desIdent == rrfIdx)){
                desVal <<= val;
            }
        }

        void addSrc(opr& inRrfIdx, opr& inVal){
            rrfIdx = inRrfIdx;
            val    = inVal;
        }

    };

    struct RsvBase;
    struct ByPassPool{

        std::vector<ByPass> _bps;
        std::vector<RsvBase*> _rsvs;

        ByPass& addByPassEle(){
            _bps.push_back(ByPass(_bps.size()));
            return *_bps.rbegin();
        }

        void addRsv(RsvBase* rsv){
            _rsvs.push_back(rsv);
        }

        void tryAssignByPassAll(Operable& desIdent, Reg& desVal){
            for (auto& bp : _bps){
                bp.tryAssignByPass(desIdent, desVal);
            }
        }

        opr& isByPassing(opr& rrfIdx){
            opr* result = &(_bps[0].valid & (rrfIdx == _bps[0].rrfIdx));
            for (int i = 1; i < _bps.size(); i++){
                result = &((*result) | (_bps[i].valid & (rrfIdx == _bps[i].rrfIdx)));
            }
            return *result;
        }

        opr& getByPassData(opr& rrfIdx){
            mWire(getBpData, DATA_LEN);
            for (ByPass& bp : _bps){
                zif(bp.valid && (bp.rrfIdx == rrfIdx)){
                    getBpData = bp.val;
                }
            }
            return getBpData;
        }

        void doByPass(ByPass& bp);

    };



    struct TagMgmt{
        BroadCast bc;
        TagGen    tagGen{bc};
        Mpft      mpft;


    };

    struct RegArch{
        Arf arf;
        Rrf rrf;
        ByPassPool bpp;
    };

    struct PipStage{
        FetchStage  ft;
        DecodeStage dc;
        DispStage   ds;
        RsvStage    rs;
        ExecStage   ex;
        BranchStage br;
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
