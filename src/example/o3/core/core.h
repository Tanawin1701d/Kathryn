//
// Created by tanawin on 2/10/25.
//

#ifndef KATHRYN_SRC_O3_CORE_H
#define KATHRYN_SRC_O3_CORE_H

#include "kathryn.h"
//////// include pipeline staage
#include "fetch.h"
#include "decoder.h"
#include "dispatch.h"
#include "execAlu.h"
#include "execMul.h"
#include "execLdSt.h"
#include "execBranch.h"

#include "rob.h"
//////// parameter
#include "slotParam.h"
//////// regmgmt + tagmgmt
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
    /////// reorder buffer
    mMod(prob, Rob, pm, regArch);
    /////// store buffer
    StoreBuf   storeBuf{pm.ldSt, tagMgmt.bc};
    /////// front-end
    mMod(pFetch,  FetchMod  , pm     , tagMgmt,
                  prob.getCom1Entry());
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
                  pm         , pFetch           , prob   ,
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
        pExAlu1   .setSimProbe(&pipProbGrp.execAlu);
        pExBra   .setSimProbe(&pipProbGrp.execBranch);

        ///// rsv operation
        rsvs.setDebugProbe();
        rsvs.buildIssues(pm, tagMgmt.bc);


    }

    };

}

#endif //KATHRYN_SRC_O3_CORE_H