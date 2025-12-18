//
// Created by tanawin on 12/12/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSVS_H
#define KATHRYN_SRC_EXAMPLE_O3_RSVS_H

#include "irsv.h"
#include "orsv.h"
#include "stageStruct.h"

namespace kathryn::o3{
    struct Rsvs{
        ORsv alu1, alu2, mul;
        IRsv br, ls;
        std::vector<RsvBase*> rsvs{&alu1, &alu2, &mul, &br, &ls};
        Rsvs(RegArch& regArch, BroadCast& bc):
        alu1(RS_ENT_ALU   , smRsvBase + smRsvAlu   , ALU_ENT_NUM   , regArch        ),
        alu2(RS_ENT_ALU   , smRsvBase + smRsvAlu   , ALU_ENT_NUM   , regArch        ),
        mul (RS_ENT_MUL   , smRsvBase + smRsvMul   , MUL_ENT_NUM   , regArch, smRsvI),
        br  (RS_ENT_BRANCH, smRsvBase + smRsvBranch, BRANCH_ENT_SEL, "br"   , bc    ),
        ls  (RS_ENT_LDST  , smRsvBase + smRsvAlu   , LDST_ENT_SEL  , "ld"   , bc    ){}

        void onMisPred(opr& fixTag){
            for (RsvBase* rsv: rsvs){
                rsv->onMisPred(fixTag);
            }
        }

        void onSucPred(opr& sucTag){
            for (RsvBase* rsv: rsvs){
                rsv->onSucPred(sucTag);
            }
        }

        void buildIssues(PipStage& pm, BroadCast& bc){
            ///// build alu reservation station issue logic
            alu1.buildIssue(pm.ex[0].sync, bc);
            alu2.buildIssue(pm.ex[1].sync, bc);
            ///// build alu reservation station issue logic
            mul.buildIssue(pm.mu.sync, bc);
            ///// build branch reservation station internal logic
            br.buildIssue(pm.br.sync, bc);
            ///// build load/store reservation station internal logic
            ls.buildIssue(pm.ldSt.sync, bc);
        }

        //////START DO NOT COUNT

        void setDebugProbe(){
            ///// todo set simprobe for all
            alu1.setSimProbe(&zyncProbGrp.issueAlu1   , &dataStructProbGrp.rsvAlu1  );
            alu2.setSimProbe(&zyncProbGrp.issueAlu2   , &dataStructProbGrp.rsvAlu2  );
            mul .setSimProbe(&zyncProbGrp.issueMul    , &dataStructProbGrp.rsvMul   );
            br  .setSimProbe(&zyncProbGrp.issueBranch , &dataStructProbGrp.rsvbranch);
            ls  .setSimProbe(&zyncProbGrp.issueLdSt   , &dataStructProbGrp.rsvLdSt  );
        }

        //////END DO NOT COUNT
    };
}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSVS_H