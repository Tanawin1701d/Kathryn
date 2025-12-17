//
// Created by tanawin on 5/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_FETCH_H
#define KATHRYN_SRC_EXAMPLE_O3_FETCH_H

#include "kathryn.h"
#include "stageStruct.h"
#include "parameter.h"
#include "example/o3/simulation/proberGrp.h"
#include "btb.h"
#include "gshare.h"


namespace kathryn::o3{

    struct FetchMod : Module{
        PipStage& pm;
        Reg&      curPc;
        Btb       btb;
        Gshare    gshare;
        WireSlot& cmSlot;

        mWire(fetchPredCond   , 1); /// from
        mWire(fetchBtbHit     , 1);
        mWire(fetchHitAndTaken, 1);
        mWire(fetchBtbAddr    , ADDR_LEN);

        explicit FetchMod(PipStage&  pm,
                          TagMgmt& tagMgmt,
                          WireSlot& commitSlot) :
        pm        (pm),
        curPc     (pm.ft.curPc),
        gshare    (tagMgmt.mpft),
        cmSlot    (commitSlot){
            curPc.makeResetEvent();
        }

        void flow(){

            ///// pipeline manager
            pip(pm.ft.sync){ autoSync     initProbe(pipProbGrp .fetch);
                zync(pm.dc.sync){          initProbe(zyncProbGrp.fetch);
                    selLog();
                }
            }


            ///// search data from btb for fetch
            std::tie(fetchBtbHit, fetchBtbAddr) =
                btb.onInquire(curPc, pm.ft.raw(invalid2));
            ///// search data from pht for fetch
            gshare.buildPhtReader(convertPcToPhtIdx(curPc         , gshare.bhrMaster),
                                  convertPcToPhtIdx(cmSlot(pc), cmSlot(bhr) ));
            fetchPredCond    = (gshare.fetPhtVal > 1);
            fetchHitAndTaken = fetchBtbHit & fetchPredCond;


        }

        opr& convertPcToPhtIdx(opr& inPc, opr& inBhr){
            return inPc.sl(GSH_BHR_ST_IDX, GSH_BHR_END_IDX) ^ inBhr;
        }

        void onSucPred(opr& sucTag){
            gshare.onSucPred_bhrUpdate(sucTag,
                                       fetchHitAndTaken);
        }

        void onMisPred(opr& misTag, opr& fixedPc){
            ///// fixed Pc
            pm.ft.incPc(fixedPc, true);
            ///// fix gshare predictor
            gshare.onMisPred_bhrUpdate(misTag);
        }


        void onBranchCommit(){
            ////// update btb data
            btb.onCommit(cmSlot(pc), cmSlot(jumpAddr));
            ////// update gshare predictor
            opr& phtAddr = convertPcToPhtIdx(cmSlot(pc), cmSlot(bhr));
            gshare.onCommit_PhtUpdate(phtAddr, cmSlot(jumpCond));
            ////// the bhr update //// I dont know why they do something like this
            gshare.onCommit_bhrUpdate(fetchBtbHit, fetchPredCond);
        }

        void selLog(){
            ///// ignore first 4 bytes, because instruction is 4 bytes long
            opr& selIdx = curPc(2, 4);

            ///// cal next pc
            opr& cal_npc    = mux(fetchHitAndTaken, fetchBtbAddr,
                              mux(selIdx.sl(0), curPc + 4,
                                  curPc + 8)
                                );
            pm.ft.incPc(cal_npc);



            ///// slot assign
            RegSlot& raw = pm.ft.raw;
            raw(invalid2) <<= selIdx.sl(0);
            raw(pc)       <<= curPc;

            raw(npc)      <<= cal_npc;
            raw(prCond)   <<= fetchHitAndTaken;
            raw(bhr)      <<= gshare.bhrMaster;



            ////// read instruction from main memory
            auto& i0 = pm.ft.iMem0;
            auto& i1 = pm.ft.iMem1;
            auto& i2 = pm.ft.iMem2;
            auto& i3 = pm.ft.iMem3;

            zif(selIdx == 0){
                raw(inst1)    <<=  i0;
                raw(inst2)    <<=  i1;
            }zelif(selIdx == 1){
                raw(inst1)    <<=  i1;
                raw(inst2)    <<=  i2;
            }zelif(selIdx == 2){
                raw(inst1)    <<=  i2;
                raw(inst2)    <<=  i3;
            }zelse{ ///// the second instruction is invalid
                raw(inst1)    <<= i3;
                raw(inst2)    <<= i0;
                raw(invalid2) <<= 1;
                raw(npc)      <<= (curPc + 4);
            }
        }
    };

}

#endif //KATHRYN_FETCH_H