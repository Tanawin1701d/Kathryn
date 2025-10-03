//
// Created by tanawin on 2/10/25.
//

#ifndef KATHRYN_SRC_O3_CORE_H
#define KATHRYN_SRC_O3_CORE_H

#include "kathryn.h"
#include "decoder.h"
#include "dispatch.h"
#include "execAlu.h"
#include "execBranch.h"
#include "slotParam.h"
#include "stageStruct.h"

namespace kathryn::o3{

    struct Core: Module{

    BroadCast  bc;
    ByPassPool bp;
    /////// reservation station
    ORsv aluRsv     {smRsvO + smRsvBase, ALU_ENT_NUM};
    IRsv branchRsv  {smRsvI + smRsvBase, BRANCH_ENT_SEL, "br", bc};
    /////// tagmgmt
    TagMgmt  tagMgmt;
    /////// register architecture
    RegArch  regArch;
    /////// reorder buffer
    mMod(prob, Rob, regArch);
    /////// pipeline manager
    PipStage pm;

    mMod(pDec , DecMod    , pm     , tagMgmt, bc        ); //// decoder
    mMod(pDisp, DpMod     , pm.dc  ,  aluRsv, branchRsv,
                            regArch,      bp, prob      ); //// dispathc
    mMod(pExec, ExecAlu   , pm.ex  , regArch            ); //// exec unit
    mMod(pbra , BranchExec, tagMgmt, regArch, pm        ); //// branch unit


    explicit Core(){

    }


    void flow() override{

        ///// build alu reservation station internal logic
        aluRsv.buildSlotLogic(bp, bc);
        auto aluSrc = aluRsv.buildIssue(pm.ex.sync, bc);
        pExec.src.copyFrom(aluSrc);
        ///// build branch reservation station internal logic
        branchRsv.buildAllocLogic(bc);
        auto branchSrc = branchRsv.buildIssue(pm.br.sync, bc);
        pbra.src.copyFrom(branchSrc);



    }



    };

}

#endif //KATHRYN_SRC_O3_CORE_H