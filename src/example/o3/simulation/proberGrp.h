//
// Created by tanawin on 14/10/25.
//

#ifndef SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H
#define SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H

#include "kathryn.h"
#include "sim/modelSimEngine/hwCollection/dataStructure/table/tableProber.h"

namespace kathryn::o3{

        struct O3PipProbGrp{
            PipSimProbe fetch;
            PipSimProbe decode;
            PipSimProbe dispatch;
            //PipSimProbe issue;
            PipSimProbe execAlu1;
            PipSimProbe execAlu2;
            PipSimProbe execMul;
            PipSimProbe execBranch;
            PipSimProbe execLdSt;
            PipSimProbe execLdSt2;

        };

        struct O3ZyncProbGrp{
            /////////// zync front end
            ZyncSimProb fetch; //// it means fetch zyncing to next pipeline stage
            ZyncSimProb decode;
            ZyncSimProb dispatch;
            //////////// zync reservation station
            ZyncSimProb issueAlu1;
            ZyncSimProb issueAlu2;
            ZyncSimProb issueMul;
            ZyncSimProb issueBranch;
            ZyncSimProb issueLdSt;  ///// issue from reservation station to ldst
            ZyncSimProb loadStore2; ///// issue from first ldst to second ldst

        };

        struct O3DataStructProbGrp{
            TableSimProbe mpft;
            TableSimProbe arfBusy;
            TableSimProbe arfRename;
            TableSimProbe rrf;
            TableSimProbe rsvAlu1;
            TableSimProbe rsvAlu2;
            TableSimProbe rsvMul;
            TableSimProbe rsvbranch;
            TableSimProbe rsvLdSt;
            TableSimProbe commit;

            void applyCycleChange(){
                mpft     .applyChange();
                arfBusy  .applyChange();
                arfRename.applyChange();
                rrf      .applyChange();
                /////// reservation station
                rsvAlu1  . applyChange();
                rsvAlu2  . applyChange();
                rsvbranch. applyChange();
                rsvMul   . applyChange();
                rsvLdSt  . applyChange();
                /////// reorder buffer
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
