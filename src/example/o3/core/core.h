//
// Created by tanawin on 2/10/25.
//

#ifndef KATHRYN_SRC_O3_CORE_H
#define KATHRYN_SRC_O3_CORE_H

//////// include pipeline stages
#include "fetch.h"
#include "decoder.h"
#include "dispatch.h"
#include "execAlu.h"
#include "execMul.h"
#include "execLdSt.h"
#include "execBranch.h"
//////// include data structure
#include "rob.h"
#include "rsvs.h"
#include "stageStruct.h"
#include "storeBuf.h"

namespace kathryn::o3{


    struct Core: Module{

        /////// tagmgmt
        TagMgmt  tagMgmt;
        /////// register architecture
        RegArch  regArch {tagMgmt.mpft};
        /////// reservation stations
        Rsvs     rsvs {regArch, tagMgmt.bc};
        /////// pipeline manager
        PipStage pm;
        /////// store buffer
        StoreBuf   storeBuf{pm.ldSt, tagMgmt.bc};
        /////// reorder buffer
        mMod(prob, Rob, pm, regArch, storeBuf);
        /////// front-end
        mMod(pFetch,  FetchMod  , pm     , tagMgmt,
                      prob.getBranchUpdateEntry());
        mMod(pDec  ,  DecMod    , pm     , tagMgmt); //// decoder
        mMod(pDisp ,  DpMod     , pm     , rsvs ,
                      regArch   , tagMgmt, prob); //// dispathc
        /////// back-end
        mMod(pExAlu1,  ExecAlu   , pm.ex[0]         , regArch,
                       prob      , rsvs.alu1.execSrc          ); //// exec
        mMod(pExAlu2,  ExecAlu   , pm.ex[1]         , regArch,
                       prob      , rsvs.alu2.execSrc          );
        mMod(pMulAlu, ExecMul    , pm.mu            , regArch,
                      prob       , rsvs.mul.execSrc           ); //// multiplier unit
        mMod(pExBra,  BranchExec , tagMgmt          , regArch,
                      pm         , pDisp  ,
                      prob       , storeBuf         ,
                      rsvs    ); //// branch unit
        mMod(pExLdSt, ExecLdSt   , pm.ldSt          , regArch,
                      tagMgmt.bc , prob             , rsvs.ls.execSrc,
                      storeBuf);


        explicit Core(int x){
            ///// add reservation to bypass and prediction control
            regArch.bpp.addRsvs(&rsvs);
        }

        void flow() override{

            ///// set sim probe for the exec unit and reservation station
            pExAlu1   .setSimProbe (&pipProbGrp.execAlu1   ); ///DC
            pExAlu2   .setSimProbe (&pipProbGrp.execAlu2   ); ///DC
            pMulAlu   .setSimProbe (&pipProbGrp.execMul    ); ///DC
            pExBra    .setSimProbe (&pipProbGrp.execBranch ); ///DC
            pExLdSt   .setSimProbe (&pipProbGrp.execLdSt   ); ///DC
            pExLdSt   .setZyncProb (&zyncProbGrp.loadStore2); ///DC
            pExLdSt   .setSimProbe2(&pipProbGrp.execLdSt2  ); ///DC

            ///// rsv operation
            rsvs.setDebugProbe(); ///DC
            rsvs.buildIssues(pm, tagMgmt.bc);
        }
    };
}

#endif //KATHRYN_SRC_O3_CORE_H