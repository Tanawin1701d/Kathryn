//
// Created by tanawin on 14/10/25.
//

#ifndef SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H
#define SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H

#include "kathryn.h"
#include "sim/modelSimEngine/hwCollection/dataStructure/table/tableProber.h"

namespace kathryn{
    namespace o3{

        struct O3PipProbGrp{
            PipSimProbe fetch;
            PipSimProbe decode;
            PipSimProbe dispatch;
            //PipSimProbe issue;
            PipSimProbe execAlu;
            PipSimProbe execBranch;
        };

        struct O3ZyncProbGrp{
            ZyncSimProb fetch; //// it means fetch zyncing to next pipeline stage
            ZyncSimProb decode;
            ZyncSimProb dispatch;
            ZyncSimProb issueAlu;
            ZyncSimProb issueBranch;
        };

        struct O3DataStructProbGrp{
            TableSimProbe mpft;
            TableSimProbe arfBusy;
            TableSimProbe arfRename;
            TableSimProbe rrf;
            TableSimProbe rsvAlu;
            TableSimProbe rsvbranch;
            TableSimProbe commit;

            void applyCycleChange(){
                mpft     .applyChange();
                arfBusy  .applyChange();
                arfRename.applyChange();
                rrf      .applyChange();
                rsvAlu   .applyChange();
                rsvbranch.applyChange();
                commit   .applyChange();
            }
        };

        // struct O3DbgWireGrp{
        //     mWire(disp_isAluRsvAllocatable, 1);
        //     mWire(disp_isBranchRsvAllocatable, 1);
        //     mWire(disp_isRenamable, 1);
        // };

        inline O3PipProbGrp        pipProbGrp;
        inline O3ZyncProbGrp       zyncProbGrp;
        inline O3DataStructProbGrp dataStructProbGrp;

        //inline O3DbgWireGrp dbgWireGrp;

    }
}

#endif //SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H
