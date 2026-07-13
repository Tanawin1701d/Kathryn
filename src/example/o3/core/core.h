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


    struct Core: Module{   ///MD CORE

        /////// tagmgmt
        TagMgmt  tagMgmt;   ///MD CORE
        /////// register architecture
        RegArch  regArch                       ///MD CORE
                 {tagMgmt.mpft};               ///CTRL_HC CORE
        /////// reservation stations
        Rsvs     rsvs                          ///MD CORE
                 {regArch, tagMgmt.bc};        ///CTRL_HC+DATA_HC CORE
        /////// pipeline manager
        PipStage pm;   ///MD CORE
        /////// store buffer
        StoreBuf storeBuf                      ///MD CORE
                 {pm.ldSt, tagMgmt.bc};        ///CTRL_HC+DATA_HC CORE
        /////// reorder buffer
        mMod(prob, Rob,                        ///MD CORE
             pm,                               ///CTRL_HC+DATA_HC CORE
             regArch, storeBuf);              ///CTRL_HC+DATA_HC CORE
        /////// front-end
        mMod(pFetch, FetchMod,                 ///MD CORE
             pm,                               ///CTRL_HC+DATA_HC CORE
             tagMgmt, prob.getBranchUpdateEntry());   ///CTRL_HC CORE
        mMod(pDec, DecMod,                     ///MD CORE
             pm,                               ///CTRL_HC+DATA_HC CORE
             tagMgmt); //// decoder            ///CTRL_HC CORE
        mMod(pDisp, DpMod,                     ///MD CORE
             pm, rsvs, regArch,                ///CTRL_HC+DATA_HC CORE
             tagMgmt,                          ///CTRL_HC CORE
             prob); //// dispathc              ///CTRL_HC+DATA_HC CORE
        /////// back-end
        mMod(pExAlu1, ExecAlu,                 ///MD CORE
             regArch,                          ///CTRL_HC+DATA_HC CORE
             prob, rsvs.alu1); //// exec       ///CTRL_HC+DATA_HC CORE
        mMod(pExAlu2, ExecAlu,                 ///MD CORE
             regArch,                          ///CTRL_HC+DATA_HC CORE
             prob, rsvs.alu2);                 ///CTRL_HC+DATA_HC CORE
        mMod(pMulAlu, ExecMul,                 ///MD CORE
             regArch,                          ///CTRL_HC+DATA_HC CORE
             prob, rsvs.mul); //// multiplier unit   ///CTRL_HC+DATA_HC CORE
        mMod(pExBra, BranchExec,               ///MD CORE
             tagMgmt,                          ///CTRL_HC CORE
             regArch,                          ///CTRL_HC+DATA_HC CORE
             pm, pDisp, prob,                  ///CTRL_HC+DATA_HC CORE
             storeBuf,                         ///CTRL_HC+DATA_HC CORE
             rsvs); //// branch unit           ///CTRL_HC+DATA_HC CORE
        mMod(pExLdSt, ExecLdSt,                ///MD CORE
             pm.ldSt,                          ///CTRL_HC+DATA_HC CORE
             regArch,                          ///CTRL_HC+DATA_HC CORE
             tagMgmt.bc,                       ///CTRL_HC CORE
             prob, rsvs.ls,                    ///CTRL_HC+DATA_HC CORE
             storeBuf);                        ///CTRL_HC+DATA_HC CORE


        explicit Core(int x){   ///HLH CORE
            ///// add reservation to bypass and prediction control
            regArch.bpp.addRsvs(&rsvs);   ///CTRL_HC+DATA_HC CORE
        }

        void flow() override{   ///HLH CORE

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
            rsvs.buildIssues(pm, tagMgmt.bc);   ///CTRL_HC+DATA_HC CORE
        }
    };
}

#endif //KATHRYN_SRC_O3_CORE_H
