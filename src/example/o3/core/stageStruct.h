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

        void incPc(opr& nextPc, bool isMisPred = false){
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER + isMisPred);
            curPc <<= nextPc;
            SET_ASM_PRI_TO_AUTO();
        }
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
        SyncMeta sync {"branchSync"};
    };

    struct LdStStage{
        SyncPip sync  {"ldStSync"};
        SyncPip sync2 {"ldStLastSync"};
        mWire(dmem_rdata, DATA_LEN);
        mWire(dmem_we , 1);
        mWire(dmem_rwaddr, ADDR_LEN); //// must mux with reading
        mWire(dmem_wdata, DATA_LEN);
        RegSlot lsRes {smLdSt};

        LdStStage(){
            dmem_rdata  .asInputGlob ("dmem_rdata");
            dmem_we     .asOutputGlob("dmem_we");
            dmem_rwaddr .asOutputGlob("dmem_rwaddr");
            dmem_wdata  .asOutputGlob("dmem_wdata");


        }

    };

    struct CommitStage{
        SyncMeta sync {"commitSync"};
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

        std::vector<ByPass*> _bps;
        std::vector<RsvBase*> _rsvs;

        ByPass& addByPassEle(){
            _bps.emplace_back(new ByPass(_bps.size()));
            return **_bps.rbegin();
        }

        ~ByPassPool(){
            for (ByPass* bp: _bps){delete bp;}
        }

        void addRsv(RsvBase* rsv){
            _rsvs.push_back(rsv);
        }

        void tryAssignByPassAll(Operable& desIdent, Reg& desVal){
            for (auto& bp : _bps){
                bp->tryAssignByPass(desIdent, desVal);
            }
        }

        opr& isByPassing(opr& rrfIdx){
            opr* result = &(_bps[0]->valid & (rrfIdx == _bps[0]->rrfIdx));
            for (int i = 1; i < _bps.size(); i++){
                result = &((*result) | (_bps[i]->valid & (rrfIdx == _bps[i]->rrfIdx)));
            }
            return *result;
        }

        void assByPassData(Wire& desWire, opr& rrfIdx){
            for (ByPass* bp : _bps){
                zif(bp->valid && (bp->rrfIdx == rrfIdx)){
                    desWire = bp->val;
                }
            }
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

        RegArch(Mpft& mpft): arf(mpft){}
    };

    struct PipStage{
        FetchStage  ft;
        DecodeStage dc;
        DispStage   ds;
        RsvStage    rs;
        ExecStage   ex;
        BranchStage br;
        LdStStage   ldSt;
        CommitStage cm;
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
