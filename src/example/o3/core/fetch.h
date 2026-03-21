//
// Created by tanawin on 5/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_FETCH_H
#define KATHRYN_SRC_EXAMPLE_O3_FETCH_H

#include "kathryn.h"
#include "stage_struct.h"
#include "parameter.h"
#include "example/o3/simulation/prober_grp.h" ///DC
#include "btb.h"
#include "gshare.h"


namespace kathryn::o3{

    struct FetchMod : Module{
        PipStage& pm;
        Reg&      cur_pc;
// #ifdef BTB_ENABLE
//         Btb       btb;
//         Gshare    gshare;
// #endif
        WireSlot& cm_slot;

        // m_wire(fetch_pred_cond   , 1); /// from
        // m_wire(fetch_btb_hit     , 1);
        // m_wire(fetch_hit_and_taken, 1);
        // m_wire(fetch_btb_addr    , ADDR_LEN);

        explicit FetchMod(PipStage&  pm,
                          TagMgmt& tag_mgmt,
                          WireSlot& commit_slot) :
        pm        (pm),
        cur_pc     (pm.ft.cur_pc),
// #ifdef BTB_ENABLE
//         gshare    (tag_mgmt.mpft),
// #endif
        cm_slot    (commit_slot){
            cur_pc.make_reset_event();
            pm.ft.raw.make_reset_event();
        }

        void flow(){
            ///// pipeline manager
            pip(pm.ft.sync){ auto_sync     init_probe(pip_prob_grp .fetch); ///CTRL FETCH
                zync(pm.dc.sync){          init_probe(zync_prob_grp.fetch);///CTRL FETCH
                    sel_log();
                }
            }
        }

// #ifdef BTB_ENABLE
//
//             ///// search data from btb for fetch
//             std::tie(fetch_btb_hit, fetch_btb_addr) =
//                 btb.on_inquire(cur_pc, pm.ft.raw(invalid2));
//             ///// search data from pht for fetch
//             gshare.build_pht_reader(convert_pc_to_pht_idx(cur_pc         , gshare.bhr_master),
//                                   convert_pc_to_pht_idx(cm_slot(pc), cm_slot(bhr) ));
//             fetch_pred_cond    = (gshare.fet_pht_val > 1);
//             fetch_hit_and_taken = fetch_btb_hit & fetch_pred_cond;
//
// #endif

//                fetch_hit_and_taken = 0;


        // opr& convert_pc_to_pht_idx(opr& in_pc, opr& in_bhr){
        //     return in_pc.sl(GSH_BHR_ST_IDX, GSH_BHR_END_IDX) ^ in_bhr;
        // }

//         void on_suc_pred(opr& suc_tag){
// #ifdef BTB_ENABLE
//
//             gshare.onSucPred_bhrUpdate(suc_tag,
//                                        fetch_hit_and_taken);
// #endif
//
//         }

//         void on_mis_pred(opr& mis_tag, opr& fixed_pc){
//             ///// fixed Pc
//             pm.ft.inc_pc(fixed_pc, true);
//             ///// fix gshare predictor
// #ifdef BTB_ENABLE
//             gshare.onMisPred_bhrUpdate(mis_tag);
// #endif
//         }


//         void on_branch_commit(){
// #ifdef BTB_ENABLE
//             ////// update btb data
//             btb.on_commit(cm_slot(pc), cm_slot(jump_addr));
//             ////// update gshare predictor
//             opr& pht_addr = convert_pc_to_pht_idx(cm_slot(pc), cm_slot(bhr));
//             gshare.onCommit_PhtUpdate(pht_addr, cm_slot(jump_cond));
//             ////// the bhr update //// I dont know why they do something like this
//             gshare.onCommit_bhrUpdate(fetch_btb_hit, fetch_pred_cond);
// #endif
//         }

        void sel_log(){
            ///// ignore first 4 bytes, because instruction is 4 bytes long
            opr& sel_idx = cur_pc(2, 4);
            ///// cal next pc
            opr& cal_npc    = mux(sel_idx.sl(0), cur_pc + 4,
                                  cur_pc + 8);
            pm.ft.inc_pc(cal_npc);
            ///// slot assign
            RegSlot& raw = pm.ft.raw;
            raw(invalid2) <<= sel_idx.sl(0);
            raw(pc)       <<= cur_pc;

            raw(npc)      <<= cal_npc;
            ////// read instruction from main memory
            auto& i0 = pm.ft.i_mem0;
            auto& i1 = pm.ft.i_mem1;
            auto& i2 = pm.ft.i_mem2;
            auto& i3 = pm.ft.i_mem3;

            raw(inst1) <<= mux(sel_idx, {&i0, &i1, &i2, &i3});
            raw(inst2) <<= mux(sel_idx, {&i1, &i2, &i3, &i0});
        }
//            raw(pr_cond)   <<= fetch_hit_and_taken;
// #ifdef BTB_ENABLE
//             raw(bhr)      <<= gshare.bhr_master;
// #else
//            raw(bhr)      <<= 0;
//#endif

            // ztate(sel_idx){
            //     zcase(0){
            //         raw(inst1)    <<=  i0;
            //         raw(inst2)    <<=  i1;
            //     }
            //     zcase(1){
            //         raw(inst1)    <<=  i1;
            //         raw(inst2)    <<=  i2;
            //     }
            //     zcase(2){
            //         raw(inst1)    <<=  i2;
            //         raw(inst2)    <<=  i3;
            //     }
            //     zcasedef{ ///// the second instruction is invalid
            //         raw(inst1)    <<= i3;
            //         raw(inst2)    <<= i0;
            //         raw(invalid2) <<= 1;
            //         raw(npc)      <<= (cur_pc + 4);
            //     }
            // }

    };

}

#endif //KATHRYN_FETCH_H