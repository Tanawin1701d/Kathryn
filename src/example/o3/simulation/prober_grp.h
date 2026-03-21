//
// Created by tanawin on 14/10/25.
//

#ifndef SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H
#define SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H

#include "kathryn.h"
#include "sim/model_sim_engine/hw_collection/data_structure/table/table_prober.h"

namespace kathryn::o3{

        struct O3PipProbGrp{
            PipSimProbe fetch;
            PipSimProbe decode;
            PipSimProbe dispatch;
            //PipSimProbe issue;
            PipSimProbe exec_alu1;
            PipSimProbe exec_alu2;
            PipSimProbe exec_mul;
            PipSimProbe exec_branch;
            PipSimProbe exec_ld_st;
            PipSimProbe exec_ld_st2;

        };

        struct O3ZyncProbGrp{
            /////////// zync front end
            ZyncSimProb fetch; //// it means fetch zyncing to next pipeline stage
            ZyncSimProb decode;
            ZyncSimProb dispatch;
            //////////// zync reservation station
            ZyncSimProb issue_alu1;
            ZyncSimProb issue_alu2;
            ZyncSimProb issue_mul;
            ZyncSimProb issue_branch;
            ZyncSimProb issue_ld_st;  ///// issue from reservation station to ldst
            ZyncSimProb load_store2; ///// issue from first ldst to second ldst

        };

        struct O3DataStructProbGrp{
            TableSimProbe mpft;
            TableSimProbe arf_busy;
            TableSimProbe arf_rename;
            TableSimProbe rrf;
            TableSimProbe rsv_alu1;
            TableSimProbe rsv_alu2;
            TableSimProbe rsv_mul;
            TableSimProbe rsvbranch;
            TableSimProbe rsv_ld_st;
            TableSimProbe commit;

            void apply_cycle_change(){
                mpft     .apply_change();
                arf_busy  .apply_change();
                arf_rename.apply_change();
                rrf      .apply_change();
                /////// reservation station
                rsv_alu1  . apply_change();
                rsv_alu2  . apply_change();
                rsvbranch. apply_change();
                rsv_mul   . apply_change();
                rsv_ld_st  . apply_change();
                /////// reorder buffer
                commit   .apply_change();
            }
        };

        // struct O3DbgWireGrp{
        //     m_wire(disp_isAluRsvAllocatable, 1);
        //     m_wire(disp_isBranchRsvAllocatable, 1);
        //     m_wire(disp_isRenamable, 1);
        // };

        inline O3PipProbGrp        pip_prob_grp;
        inline O3ZyncProbGrp       zync_prob_grp;
        inline O3DataStructProbGrp data_struct_prob_grp;

        //inline O3DbgWireGrp dbg_wire_grp;


}

#endif //SRC_EXAMPLE_O3_SIMULATION_PROBERGRP_H
