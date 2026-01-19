//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H

#include "parameter.h"
#include "slotParam.h"

#include "tagGen.h"
#include "mpft.h"

#include "arf.h"
#include "broadCast.h"
#include "rrf.h"
#include "syncMetaPip.h"
#include "isaParam.h"

namespace kathryn::o3{

    struct FetchStage{
        mReg (curPc, ADDR_LEN);
        mWire(iMem0, DATA_LEN);
        mWire(iMem1, DATA_LEN);
        mWire(iMem2, DATA_LEN);
        mWire(iMem3, DATA_LEN);

        SlotMeta meta{smFetch};
        RegSlot  raw {smFetch};

        SyncMeta sync    {"fetchSync"};


        FetchStage(){
            curPc.asOutputGlob("curPc");
            iMem0.asInputGlob ("iMem0");
            iMem1.asInputGlob ("iMem1");
            iMem2.asInputGlob ("iMem2");
            iMem3.asInputGlob ("iMem3");

        }

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

        SyncMeta sync    {"decodeSync"};

        Operable& getIsAlocRsv(RegSlot& dcd){ return dcw1(rsIdx_1); }

    };

    struct LdStStage{
        mWire(dmem_rdata, DATA_LEN);
        mWire(dmem_we , 1);
        mWire(dmem_rwaddr, ADDR_LEN); //// must mux with reading
        mWire(dmem_wdata, DATA_LEN);
        RegSlot lsRes {smLdSt};
        SyncPip  sync2 {"ldStLastSync"};

        LdStStage(){
            dmem_rdata  .asInputGlob ("dmem_rdata");
            dmem_we     .asOutputGlob("dmem_we");
            dmem_rwaddr .asOutputGlob("dmem_rwaddr");
            dmem_wdata  .asOutputGlob("dmem_wdata");


        }

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

    struct Rsvs;
    struct ByPassPool{

        std::vector<ByPass*>  _bps;
        Rsvs*                 _rsvs = nullptr;

        ByPass& addByPassEle(){
            _bps.emplace_back(new ByPass(_bps.size()));
            return **_bps.rbegin();
        }

        ~ByPassPool(){ for (ByPass* bp: _bps){delete bp;}}

        void addRsvs(Rsvs* rsvs){
            _rsvs = rsvs;
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
        LdStStage   ldSt;

        SyncMeta sync_dp    {"dispSync"};
        SyncMeta sync_rs    {"rsvSync"};

        SyncMeta sync_cm    {"commitSync"};


        void onMisPred(){
            ////// kill the in-order stage
            ft.sync.killSlave(true);
            dc.sync.killSlave(true);
            sync_dp.killSlave(true);
            sync_cm.holdSlave();
            ////// kill the out-of-order exec Unit stage
            //sync_ex1  .killIfTagMet(true, fixTag);
            //sync_ex2  .killIfTagMet(true, fixTag);
            //sync_mul  .killIfTagMet(true, fixTag);
            //ldSt.sync1.killIfTagMet(true, fixTag);
            //ldSt.sync2.killIfTagMet(true, fixTag);
            ////// hold reservation station to exection unit
            // sync_ex1  .holdMaster();
            // sync_ex2  .holdMaster();
            // sync_mul  .holdMaster();
            // sync_br   .holdMaster();
            // ldSt.sync1.holdMaster();
            ///ldSt .sync2.holdMaster(); //// because the master is not reservation station
            ////// hold commit to not


        }
        void onSucPred(){
            dc.sync.holdMaster(); //// hold fetch <-> decode
            sync_dp.holdMaster(); //// hold decode <-> dispatch to generate tag, but allowing system to enter decode state
            sync_rs.holdMaster(); //// hold dispatch <-> reservation station
        }


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
