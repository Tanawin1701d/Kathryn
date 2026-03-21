//
// Created by tanawin on 12/12/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSVS_H
#define KATHRYN_SRC_EXAMPLE_O3_RSVS_H

#include "irsv.h"
#include "orsv.h"

namespace kathryn::o3{
    struct Rsvs{

        std::vector<std::string> mul_ex_field  = {pc, alu_op, rsSel_1,  rsSel_2};
        std::vector<std::string> br_ex_field   = {rsSel_1, rsSel_2};
        std::vector<std::string> ld_st_ex_field = {alu_op, rsSel_1, rsSel_2};

        ORsv alu1, alu2, mul;
        IRsv br, ls;
        std::vector<RsvBase*> rsvs{&alu1, &alu2, &mul, &br, &ls};


        Rsvs(RegArch& reg_arch, BroadCast& bc):
        alu1(RS_ENT_ALU   , sm_rsv_base + sm_rsv_alu                 , ALU_ENT_NUM   , reg_arch        ),
        alu2(RS_ENT_ALU   , sm_rsv_base + sm_rsv_alu                 , ALU_ENT_NUM   , reg_arch        ),
        mul (RS_ENT_MUL   , sm_rsv_base + sm_rsv_mul    - mul_ex_field , MUL_ENT_NUM   , reg_arch, sm_rsv_i),
        br  (RS_ENT_BRANCH, sm_rsv_base + sm_rsv_branch - br_ex_field  , BRANCH_ENT_SEL, "br"   , bc    ),
        ls  (RS_ENT_LDST  , sm_rsv_base + sm_rsv_alu    - ld_st_ex_field, LDST_ENT_SEL  , "ld"   , bc    ){}

        void on_mis_pred(opr& fix_tag){
            for (RsvBase* rsv: rsvs){
                rsv->on_mis_pred(fix_tag);
                rsv->sync.hold_master();
            }
            ls.sync.kill_if_tag_met(true, fix_tag);
        }

        void on_suc_pred(opr& suc_tag){
            for (RsvBase* rsv: rsvs){
                rsv->on_suc_pred(suc_tag);
            }
        }

        void build_issues(PipStage& pm, BroadCast& bc){
            ///// build alu reservation station issue logic
            alu1.build_issue(bc);
            alu2.build_issue(bc);
            ///// build alu reservation station issue logic
            mul.build_issue(bc);
            ///// build branch reservation station internal logic
            br.build_issue(bc);
            ///// build load/store reservation station internal logic
            ls.build_issue(bc);
        }

        void set_debug_probe(){ ///DC
            ///// todo set simprobe for all
            alu1.set_sim_probe(&zync_prob_grp.issue_alu1   , &data_struct_prob_grp.rsv_alu1  ); ///DC
            alu2.set_sim_probe(&zync_prob_grp.issue_alu2   , &data_struct_prob_grp.rsv_alu2  ); ///DC
            mul .set_sim_probe(&zync_prob_grp.issue_mul    , &data_struct_prob_grp.rsv_mul   ); ///DC
            br  .set_sim_probe(&zync_prob_grp.issue_branch , &data_struct_prob_grp.rsvbranch); ///DC
            ls  .set_sim_probe(&zync_prob_grp.issue_ld_st   , &data_struct_prob_grp.rsv_ld_st  ); ///DC
        } ///DC

    };
}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSVS_H