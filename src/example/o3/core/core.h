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
#include "execBranch.h"
#include "rob.h"
//////// parameter
#include "slotParam.h"
//////// regmgmt + tagmgmt
#include "stageStruct.h"

namespace kathryn::o3{

    struct Core: Module{

    /////// tagmgmt
    TagMgmt  tagMgmt;
    /////// register architecture
    RegArch  regArch{tagMgmt.mpft};
    /////// reservation station
    ORsv aluRsv     {smRsvBase, ALU_ENT_NUM, regArch};
    IRsv branchRsv  {smRsvBase + smRsvBranch, BRANCH_ENT_SEL, "br", tagMgmt.bc};

    /////// pipeline manager
    PipStage pm;
    /////// reorder buffer
    mMod(prob, Rob, pm, regArch);

    mMod(pFetch, FetchMod  , pm);
    mMod(pDec  , DecMod    , pm     , tagMgmt); //// decoder
    mMod(pDisp , DpMod     , pm     , aluRsv
               , branchRsv , regArch, tagMgmt,  prob  ); //// dispathc
    mMod(pExAlu, ExecAlu   , pm.ex  , regArch,
                 prob      , aluRsv.execSrc  ); //// exec unit
    mMod(pExBra, BranchExec, tagMgmt, regArch,
                 pm        , prob   , branchRsv.execSrc); //// branch unit


    explicit Core(int x){
        regArch.bpp.addRsv(&aluRsv);
        regArch.bpp.addRsv(&branchRsv);
    }

    void flow() override{

        ///// set sim probe for the exec unit and reservation station
        pExAlu   .setSimProbe(&pipProbGrp.execAlu);
        pExBra   .setSimProbe(&pipProbGrp.execBranch);
        aluRsv   .setSimProbe(&zyncProbGrp.issueAlu, &dataStructProbGrp.rsvAlu);
        branchRsv.setSimProbe(&zyncProbGrp.issueBranch, &dataStructProbGrp.rsvbranch);

        ///// build alu reservation station issue logic
        aluRsv.buildIssue(pm.ex.sync, tagMgmt.bc);
        ///// build branch reservation station internal logic
        branchRsv.buildIssue(pm.br.sync, tagMgmt.bc);

    }

    };

}

#endif //KATHRYN_SRC_O3_CORE_H