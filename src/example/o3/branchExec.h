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

    struct BranchExec: Module{

        RegSlot     src;
        TagMgmt&    tagMgmt;
        RegArch&    regArch;
        PipStageRef pr;


        explicit BranchExec(RegSlot srcData,
                            TagMgmt& tagMgmt,
                            RegArch& regArch,
                            PipStageRef& pipRef) :
            src(srcData),
            tagMgmt(tagMgmt),
            regArch(regArch),
            pr(pipRef)
        {}

        void flow() override{

            pip(pr.br.sync){
                rob.onWriteBack(src(rrftag));
                /////// do about the decode
                zif(src(rdUse)){
                    rrf.onWback(src(rrftag), src(pc+4));
                }

                zif (){ //// case miss predict
                    ////// kill the pipeline stage
                    pr.ft.sync.killSlave();
                    pr.dc.sync.killSlave();
                    pr.ds.sync.killSlave();
                    pr.ex.sync.killSlave(tagMgmt.mpft.);
                    ////// do recovery on the system
                    tagMgmt.mpft.onMissPred();
                    tagMgmt.tagGen.onMisPred();
                    regArch.arf.onMisPred();
                    regArch.rob.onMisPred();
                    regArch.rrf.onMisPred(src(rrftag),
                        regArch.rob.getComPtr());

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

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BRANCHEXEC_H
