//
// Created by tanawin on 12/12/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSVS_H
#define KATHRYN_SRC_EXAMPLE_O3_RSVS_H

#include "irsv.h"
#include "orsv.h"

namespace kathryn::o3{
    struct Rsvs{   ///MD RSV_SHARED

        std::vector<std::string> mulExField  = {pc, aluOp, rsSel_1,  rsSel_2};   ///HLH RSV_SHARED
        std::vector<std::string> brExField   = {rsSel_1, rsSel_2};               ///HLH RSV_SHARED
        std::vector<std::string> ldStExField = {aluOp, rsSel_1, rsSel_2};        ///HLH RSV_SHARED

        ORsv alu1, alu2, mul;                                       ///MD RSV_SHARED
        IRsv br, ls;                                                ///MD RSV_SHARED
        std::vector<RsvBase*> rsvs{&alu1, &alu2, &mul, &br, &ls};   ///HLH RSV_SHARED


        Rsvs(RegArch& regArch, BroadCast& bc):                                                          ///CTRL_HC+DATA_HC RSV_SHARED
        alu1(RS_ENT_ALU   , smRsvBase + smRsvAlu                 , ALU_ENT_NUM   , regArch        ),    ///CTRL_HC+DATA_HC RSV_SHARED
        alu2(RS_ENT_ALU   , smRsvBase + smRsvAlu                 , ALU_ENT_NUM   , regArch        ),    ///CTRL_HC+DATA_HC RSV_SHARED
        mul (RS_ENT_MUL   , smRsvBase + smRsvMul    - mulExField , MUL_ENT_NUM   , regArch, smRsvI),    ///CTRL_HC+DATA_HC RSV_SHARED
        br  (RS_ENT_BRANCH, smRsvBase + smRsvBranch - brExField  , BRANCH_ENT_SEL, "br"   , bc    ),    ///CTRL_HC RSV_SHARED
        ls  (RS_ENT_LDST  , smRsvBase + smRsvAlu    - ldStExField, LDST_ENT_SEL  , "ld"   , bc    ){}   ///CTRL_HC RSV_SHARED

        void onMisPred(opr& fixTag){              ///CTRL_HC RSV_SHARED
            for (RsvBase* rsv: rsvs){             ///HLH RSV_SHARED
                rsv->onMisPred(fixTag);           ///CTRL_HC RSV_SHARED
                rsv->sync.holdMaster();           ///CTRL_CL RSV_SHARED
            }
            ls.sync.killIfTagMet(true, fixTag);   ///CTRL_CL RSV_SHARED
        }

        void onSucPred(opr& sucTag){      ///CTRL_HC RSV_SHARED
            for (RsvBase* rsv: rsvs){     ///HLH RSV_SHARED
                rsv->onSucPred(sucTag);   ///CTRL_HC RSV_SHARED
            }
        }

        void buildIssues(PipStage& pm, BroadCast& bc){   ///CTRL_HC+DATA_HC RSV_SHARED
            ///// build alu reservation station issue logic
            alu1.buildIssue(bc);                         ///CTRL_HC RSV_SHARED
            alu2.buildIssue(bc);                         ///CTRL_HC RSV_SHARED
            ///// build alu reservation station issue logic
            mul.buildIssue(bc);                          ///CTRL_HC RSV_SHARED
            ///// build branch reservation station internal logic
            br.buildIssue(bc);                           ///CTRL_HC RSV_SHARED
            ///// build load/store reservation station internal logic
            ls.buildIssue(bc);                           ///CTRL_HC RSV_SHARED
        }

        void setDebugProbe(){ ///DC
            ///// todo set simprobe for all
            alu1.setSimProbe(&zyncProbGrp.issueAlu1   , &dataStructProbGrp.rsvAlu1  ); ///DC
            alu2.setSimProbe(&zyncProbGrp.issueAlu2   , &dataStructProbGrp.rsvAlu2  ); ///DC
            mul .setSimProbe(&zyncProbGrp.issueMul    , &dataStructProbGrp.rsvMul   ); ///DC
            br  .setSimProbe(&zyncProbGrp.issueBranch , &dataStructProbGrp.rsvbranch); ///DC
            ls  .setSimProbe(&zyncProbGrp.issueLdSt   , &dataStructProbGrp.rsvLdSt  ); ///DC
        } ///DC

    };
}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSVS_H
