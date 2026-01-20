//
// Created by tanawin on 5/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_FETCH_H
#define KATHRYN_SRC_EXAMPLE_O3_FETCH_H

#include "kathryn.h"
#include "stageStruct.h"
#include "parameter.h"
#include "example/o3/simulation/proberGrp.h" ///DC
#include "btb.h"
#include "gshare.h"


namespace kathryn::o3{

    struct FetchMod : Module{
        PipStage& pm;
        Reg&      curPc;
// #ifdef BTB_ENABLE
//         Btb       btb;
//         Gshare    gshare;
// #endif
        WireSlot& cmSlot;

        // mWire(fetchPredCond   , 1); /// from
        // mWire(fetchBtbHit     , 1);
        // mWire(fetchHitAndTaken, 1);
        // mWire(fetchBtbAddr    , ADDR_LEN);

        explicit FetchMod(PipStage&  pm,
                          TagMgmt& tagMgmt,
                          WireSlot& commitSlot) :
        pm        (pm),
        curPc     (pm.ft.curPc),
// #ifdef BTB_ENABLE
//         gshare    (tagMgmt.mpft),
// #endif
        cmSlot    (commitSlot){
            curPc.makeResetEvent();
        }

        void flow(){
            ///// pipeline manager
            pip(pm.ft.sync){ autoSync     initProbe(pipProbGrp .fetch); ///CTRL FETCH
                zync(pm.dc.sync){          initProbe(zyncProbGrp.fetch);///CTRL FETCH
                    selLog();
                }
            }
        }

// #ifdef BTB_ENABLE
//
//             ///// search data from btb for fetch
//             std::tie(fetchBtbHit, fetchBtbAddr) =
//                 btb.onInquire(curPc, pm.ft.raw(invalid2));
//             ///// search data from pht for fetch
//             gshare.buildPhtReader(convertPcToPhtIdx(curPc         , gshare.bhrMaster),
//                                   convertPcToPhtIdx(cmSlot(pc), cmSlot(bhr) ));
//             fetchPredCond    = (gshare.fetPhtVal > 1);
//             fetchHitAndTaken = fetchBtbHit & fetchPredCond;
//
// #endif

//                fetchHitAndTaken = 0;


        // opr& convertPcToPhtIdx(opr& inPc, opr& inBhr){
        //     return inPc.sl(GSH_BHR_ST_IDX, GSH_BHR_END_IDX) ^ inBhr;
        // }

//         void onSucPred(opr& sucTag){
// #ifdef BTB_ENABLE
//
//             gshare.onSucPred_bhrUpdate(sucTag,
//                                        fetchHitAndTaken);
// #endif
//
//         }

//         void onMisPred(opr& misTag, opr& fixedPc){
//             ///// fixed Pc
//             pm.ft.incPc(fixedPc, true);
//             ///// fix gshare predictor
// #ifdef BTB_ENABLE
//             gshare.onMisPred_bhrUpdate(misTag);
// #endif
//         }


//         void onBranchCommit(){
// #ifdef BTB_ENABLE
//             ////// update btb data
//             btb.onCommit(cmSlot(pc), cmSlot(jumpAddr));
//             ////// update gshare predictor
//             opr& phtAddr = convertPcToPhtIdx(cmSlot(pc), cmSlot(bhr));
//             gshare.onCommit_PhtUpdate(phtAddr, cmSlot(jumpCond));
//             ////// the bhr update //// I dont know why they do something like this
//             gshare.onCommit_bhrUpdate(fetchBtbHit, fetchPredCond);
// #endif
//         }

        void selLog(){
            ///// ignore first 4 bytes, because instruction is 4 bytes long
            opr& selIdx = curPc(2, 4);
            ///// cal next pc
            opr& cal_npc    = mux(selIdx.sl(0), curPc + 4,
                                  curPc + 8);
            pm.ft.incPc(cal_npc);
            ///// slot assign
            RegSlot& raw = pm.ft.raw;
            raw(invalid2) <<= selIdx.sl(0);
            raw(pc)       <<= curPc;

            raw(npc)      <<= cal_npc;
            ////// read instruction from main memory
            auto& i0 = pm.ft.iMem0;
            auto& i1 = pm.ft.iMem1;
            auto& i2 = pm.ft.iMem2;
            auto& i3 = pm.ft.iMem3;

            raw(inst1) <<= mux(selIdx, {&i0, &i1, &i2, &i3});
            raw(inst2) <<= mux(selIdx, {&i1, &i2, &i3, &i0});
        }
//            raw(prCond)   <<= fetchHitAndTaken;
// #ifdef BTB_ENABLE
//             raw(bhr)      <<= gshare.bhrMaster;
// #else
//            raw(bhr)      <<= 0;
//#endif

            // ztate(selIdx){
            //     zcase(0){
            //         raw(inst1)    <<=  i0;
            //         raw(inst2)    <<=  i1;
            //     }
            //     zcase(1){
            //         raw(inst1)    <<=  i1;
            //         raw(inst2)    <<=  i2;
            //     }
            //     zcase(2){
            //         raw(inst1)    <<=  i2;
            //         raw(inst2)    <<=  i3;
            //     }
            //     zcasedef{ ///// the second instruction is invalid
            //         raw(inst1)    <<= i3;
            //         raw(inst2)    <<= i0;
            //         raw(invalid2) <<= 1;
            //         raw(npc)      <<= (curPc + 4);
            //     }
            // }

    };

}

#endif //KATHRYN_FETCH_H