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

    struct FetchStage{            ///MD CORE
        mReg (curPc, ADDR_LEN);   ///DATA_HWD CORE
        mWire(iMem0, DATA_LEN);   ///DATA_HWD CORE
        mWire(iMem1, DATA_LEN);   ///DATA_HWD CORE
        mWire(iMem2, DATA_LEN);   ///DATA_HWD CORE
        mWire(iMem3, DATA_LEN);   ///DATA_HWD CORE

        SlotMeta meta{smFetch};   ///DATA_HWD CORE
        RegSlot  raw {smFetch};   ///DATA_HWD CORE

        SyncMeta sync    {"fetchSync"};   ///MD CORE


        FetchStage(){                      ///HLH CORE
            curPc.asOutputGlob("curPc");   ///DATA_HC CORE
            iMem0.asInputGlob ("iMem0");   ///DATA_HC CORE
            iMem1.asInputGlob ("iMem1");   ///DATA_HC CORE
            iMem2.asInputGlob ("iMem2");   ///DATA_HC CORE
            iMem3.asInputGlob ("iMem3");   ///DATA_HC CORE

        }

        void incPc(opr& nextPc, bool isMisPred = false){              ///DATA_HC CORE
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER + isMisPred);   ///CTRL_CL CORE
            curPc <<= nextPc;                                         ///DATA_DT CORE
            SET_ASM_PRI_TO_AUTO();                                    ///CTRL_CL CORE
        }
    };

    struct DecodeStage{                      ///MD CORE
        SlotMeta sharedMeta  {smDecShard};   ///CTRL_HWD+DATA_HWD CORE
        SlotMeta decodedMeta {smDecBase };   ///CTRL_HWD+DATA_HWD CORE

        RegSlot   dcd1    {decodedMeta};     ///CTRL_HWD+DATA_HWD CORE
        RegSlot   dcd2    {decodedMeta};     ///CTRL_HWD+DATA_HWD CORE
        WireSlot  dcw1    {decodedMeta};     ///CTRL_HWD+DATA_HWD CORE
        WireSlot  dcw2    {decodedMeta};     ///CTRL_HWD+DATA_HWD CORE
        RegSlot  dcdShared{sharedMeta};      ///CTRL_HWD+DATA_HWD CORE

        SyncMeta sync    {"decodeSync"};   ///MD CORE

        Operable& getIsAlocRsv(RegSlot& dcd){ return dcw1(rsIdx_1); }   ///CTRL_HC+DATA_HC CORE

    };

    struct LdStStage{                                              ///MD CORE
        mWire(dmem_rdata, DATA_LEN);                               ///DATA_HWD CORE
        mWire(dmem_we , 1);                                        ///CTRL_HWD CORE
        mWire(dmem_rwaddr, ADDR_LEN); //// must mux with reading   ///DATA_HWD CORE
        mWire(dmem_wdata, DATA_LEN);                               ///DATA_HWD CORE
        RegSlot lsRes {smLdSt};                                    ///CTRL_HWD CORE
        SyncPip  sync2 {"ldStLastSync"};                           ///MD CORE

        LdStStage(){   ///HLH CORE
            dmem_rdata  .asInputGlob ("dmem_rdata");    ///DATA_HC CORE
            dmem_we     .asOutputGlob("dmem_we");       ///CTRL_HC CORE
            dmem_rwaddr .asOutputGlob("dmem_rwaddr");   ///DATA_HC CORE
            dmem_wdata  .asOutputGlob("dmem_wdata");    ///DATA_HC CORE


        }

    };

    struct ByPass{                ///MD CORE
        int bpIdx = -1;           ///HLH CORE
        mWire(valid, 1);          ///CTRL_HWD CORE
        mWire(rrfIdx, RRF_SEL);   ///CTRL_HWD CORE
        mWire(val, DATA_LEN);     ///DATA_HWD CORE

        ByPass(int bpIdx):bpIdx(bpIdx){}   ///HLH CORE

        void tryAssignByPass(Operable& desIdent, Reg& desVal){   ///CTRL_HC+DATA_HC CORE
            zif(valid && (desIdent == rrfIdx)){   ///CTRL_CL CORE
                desVal <<= val;   ///DATA_DT CORE
            }
        }

        void addSrc(opr& inRrfIdx, opr& inVal){   ///CTRL_HC+DATA_HC CORE
            rrfIdx = inRrfIdx;                    ///CTRL_DT CORE
            val    = inVal;                       ///DATA_DT CORE
        }

    };

    struct Rsvs;         ///HLH CORE
    struct ByPassPool{   ///MD CORE

        std::vector<ByPass*>  _bps;   ///HLH CORE
        Rsvs*                 _rsvs = nullptr;   ///CTRL_HC+DATA_HC CORE

        ByPass& addByPassEle(){                           ///CTRL_HC+DATA_HC CORE
            _bps.emplace_back(new ByPass(_bps.size()));   ///HLH CORE
            return **_bps.rbegin();                       ///DATA_HC CORE
        }

        ~ByPassPool(){ for (ByPass* bp: _bps){delete bp;}}   ///HLH CORE

        void addRsvs(Rsvs* rsvs){   ///CTRL_HC+DATA_HC CORE
            _rsvs = rsvs;   ///HLH CORE
        }

        opr& isByPassing(opr& rrfIdx){   ///CTRL_HC CORE
            opr* result = &(_bps[0]->valid & (rrfIdx == _bps[0]->rrfIdx));   ///CTRL_CL CORE
            for (int i = 1; i < _bps.size(); i++){   ///HLH CORE
                result = &((*result) | (_bps[i]->valid & (rrfIdx == _bps[i]->rrfIdx)));   ///CTRL_CL CORE
            }
            return *result;   ///CTRL_HC CORE
        }

        void assByPassData(Wire& desWire, opr& rrfIdx){   ///CTRL_HC+DATA_HC CORE
            for (ByPass* bp : _bps){   ///HLH CORE
                zif(bp->valid && (bp->rrfIdx == rrfIdx)){   ///CTRL_CL CORE
                    desWire = bp->val;   ///DATA_DT CORE
                }
            }
        }

        void doByPass(ByPass& bp);   ///CTRL_HC+DATA_HC CORE

    };



    struct TagMgmt{   ///MD CORE
        BroadCast bc;   ///MD CORE
        TagGen    tagGen   ///MD CORE
                  {bc};   ///CTRL_HC CORE
        Mpft      mpft;   ///MD CORE


    };

    struct RegArch{   ///MD CORE
        Arf arf;   ///MD CORE
        Rrf rrf;   ///MD CORE
        ByPassPool bpp;   ///MD CORE

        RegArch(Mpft& mpft): arf(mpft){}   ///CTRL_HC CORE
    };

    struct PipStage{   ///MD CORE
        FetchStage  ft;   ///MD CORE
        DecodeStage dc;   ///MD CORE
        LdStStage   ldSt;   ///MD CORE

        SyncMeta sync_dp    {"dispSync"};   ///MD CORE
        SyncMeta sync_rs    {"rsvSync"};   ///MD CORE

        SyncMeta sync_cm    {"commitSync"};   ///MD CORE


        void onMisPred(){   ///HLH CORE
            ////// kill the in-order stage
            ft.sync.killSlave(true);   ///CTRL_CL CORE
            dc.sync.killSlave(true);   ///CTRL_CL CORE
            sync_dp.killSlave(true);   ///CTRL_CL CORE
            sync_cm.holdSlave();   ///CTRL_CL CORE



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
        void onSucPred(){   ///HLH CORE
            dc.sync.holdMaster(); //// hold fetch <-> decode   ///CTRL_CL CORE
            sync_dp.holdMaster(); //// hold decode <-> dispatch to generate tag, but allowing system to enter decode state   ///CTRL_CL CORE
            sync_rs.holdMaster(); //// hold dispatch <-> reservation station   ///CTRL_CL CORE
        }


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
