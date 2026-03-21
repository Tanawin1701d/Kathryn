//
// Created by tanawin on 2/10/25.
//

#ifndef KATHRYN_SRC_O3_CORE_H
#define KATHRYN_SRC_O3_CORE_H

//////// include pipeline stages
#include "fetch.h"
#include "decoder.h"
#include "dispatch.h"
#include "exec_alu.h"
#include "exec_mul.h"
#include "exec_ld_st.h"
#include "exec_branch.h"
//////// include data structure
#include "rob.h"
#include "rsvs.h"
#include "stage_struct.h"
#include "store_buf.h"

namespace kathryn::o3{


    struct Core: Module{

        /////// tagmgmt
        TagMgmt  tag_mgmt;
        /////// register architecture
        RegArch  reg_arch {tag_mgmt.mpft};
        /////// reservation stations
        Rsvs     rsvs {reg_arch, tag_mgmt.bc};
        /////// pipeline manager
        PipStage pm;
        /////// store buffer
        StoreBuf   store_buf{pm.ld_st, tag_mgmt.bc};
        /////// reorder buffer
        m_mod(prob, Rob, pm, reg_arch, store_buf);
        /////// front-end
        m_mod(p_fetch,  FetchMod  , pm     , tag_mgmt,
                      prob.get_branch_update_entry());
        m_mod(p_dec  ,  DecMod    , pm     , tag_mgmt); //// decoder
        m_mod(p_disp ,  DpMod     , pm     , rsvs ,
                      reg_arch   , tag_mgmt, prob); //// dispathc
        /////// back-end
        m_mod(p_ex_alu1,  ExecAlu   , reg_arch, prob    , rsvs.alu1); //// exec
        m_mod(p_ex_alu2,  ExecAlu   , reg_arch, prob    , rsvs.alu2);
        m_mod(p_mul_alu, ExecMul    , reg_arch, prob    , rsvs.mul ); //// multiplier unit
        m_mod(p_ex_bra,  BranchExec , tag_mgmt, reg_arch ,
                                   pm     , p_disp   ,
                                   prob   , store_buf, rsvs    ); //// branch unit
        m_mod(p_ex_ld_st, ExecLdSt   , pm.ld_st, reg_arch , tag_mgmt.bc,
                                   prob   , rsvs.ls , store_buf);


        explicit Core(int x){
            ///// add reservation to bypass and prediction control
            reg_arch.bpp.add_rsvs(&rsvs);
        }

        void flow() override{

            ///// set sim probe for the exec unit and reservation station
            p_ex_alu1   .set_sim_probe (&pip_prob_grp.exec_alu1   ); ///DC
            p_ex_alu2   .set_sim_probe (&pip_prob_grp.exec_alu2   ); ///DC
            p_mul_alu   .set_sim_probe (&pip_prob_grp.exec_mul    ); ///DC
            p_ex_bra    .set_sim_probe (&pip_prob_grp.exec_branch ); ///DC
            p_ex_ld_st   .set_sim_probe (&pip_prob_grp.exec_ld_st   ); ///DC
            p_ex_ld_st   .set_zync_prob (&zync_prob_grp.load_store2); ///DC
            p_ex_ld_st   .set_sim_probe2(&pip_prob_grp.exec_ld_st2  ); ///DC

            ///// rsv operation
            rsvs.set_debug_probe(); ///DC
            rsvs.build_issues(pm, tag_mgmt.bc);
        }
    };
}

#endif //KATHRYN_SRC_O3_CORE_H