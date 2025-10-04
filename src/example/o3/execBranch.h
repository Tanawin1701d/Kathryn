//
// Created by tanawin on 1/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H
#define KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H

#include "kathryn.h"
#include "parameter.h"
#include "rob.h"
#include "slotParam.h"
#include "stageStruct.h"


namespace kathryn::o3{

    struct RsvBase;
    struct BranchExec: Module{

        TagMgmt&     tagMgmt;
        RegArch&     regArch;
        PipStage&    pm;
        Rob&         rob;
        RegSlot&      src;
        std::vector<RsvBase*> rsvs;


        explicit BranchExec(TagMgmt& tagMgmt,
                            RegArch& regArch,
                            PipStage& pm,
                            Rob& rob,
                            RegSlot& src) :
        tagMgmt(tagMgmt),
        regArch(regArch),
        pm(pm),
        rob(rob),
        src(src){}

        void flow() override{

            pip(pr.br.sync){
                rob.onWriteBack(src(rrftag));
                /////// do about the decode
                zif(src(rdUse)){
                    rrf.onWback(src(rrftag), src(pc+4));
                }

                zif (){ //// case miss predict

                }zelse{ //////// case success predict
                    pr.dc.sync.holdSlave(); //// because mpft and tag generator must be hold
                    pr.ds.sync.holdSlave();
                    tagMgmt.mpft.onPredSuc(src(specTag));
                    tagMgmt.tagGen.onSucPred();
                    regArch.arf.onSucPred();
                    /// no for rob and arf

                }


            }
        }

        void onMisPred();

        void onSucPred();

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H
