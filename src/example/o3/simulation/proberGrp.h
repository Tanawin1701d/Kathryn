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
            PipSimProbe wb;
            PipSimProbe commit;
        };

        struct O3ZyncProbGrp{
            ZyncSimProb fetch; //// it means fetch zyncing to next pipeline stage
            ZyncSimProb decode;
            ZyncSimProb dispatch;
            ZyncSimProb issueAlu;
            ZyncSimProb issueBranch;
            ZyncSimProb execAlu;
            ZyncSimProb execBranch;
            ZyncSimProb wb;
            ZyncSimProb commit;
        };

        struct O3DataStructProbGrp{
            TableSimProbe mpft;
            TableSimProbe arfBusy;
            TableSimProbe arfRename;
            TableSimProbe rrf;
            TableSimProbe rsvAlu;
            TableSimProbe rsvbranch;
            TableSimProbe commit;
        };

        inline O3PipProbGrp        pipProbGrp;
        inline O3ZyncProbGrp       zyncProbGrp;
        inline O3DataStructProbGrp dataStructProbGrp;

    }
}

#endif //SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H
