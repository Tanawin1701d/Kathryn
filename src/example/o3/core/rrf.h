//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_EXAMPLE_O3_RRF_H
#define KATHRYN_EXAMPLE_O3_RRF_H

#include "kathryn.h"
#include "slot_param.h"


namespace kathryn::o3{

    static int RRF_RENAME_PRI = DEFAULT_UE_PRI_USER + 1;

    struct PhyEntry{
        opr& valid;
        opr& data;
    };

    struct Rrf{

        Table table;
        m_reg(freenum, RRF_SEL + 1);
        m_reg(req_ptr,  RRF_SEL);
        m_reg(next_rrf_cycle, 1);
        m_wire(rename_req_size, 2);
        m_wire(commit_req_size, 2);
        m_val(RRF_ENTRIES, RRF_SEL + 1, RRF_NUM);


        Rrf():
        table(sm_rrf, RRF_NUM){
            table        .make_col_reset_event(rrf_valid, 0);
            freenum      .make_reset_event(RRF_NUM);
            req_ptr       .make_reset_event();
            next_rrf_cycle .make_reset_event();
            next_rrf_cycle .make_def_event();

            data_struct_prob_grp.rrf.init(&table); ///DC
        }

        opr& is_renamable(opr& req2){
            return (freenum + commit_req_size) >= (req2.uext(2) + 1);
        }

        Operable& get_req_ptr(){return req_ptr;}

        PhyEntry get_phy_entry(opr& rrf_idx){
            auto agent = table[rrf_idx];
            return {agent(rrf_valid).v(),
                    agent(rrf_data ).v()};
        }

        opr& get_phy_data(opr& rrf_idx){
            return table[rrf_idx](rrf_data).v();
        }

        //// fix_rrf is the idx who must walk away the size support to be rrf
        void on_mis_pred(opr& mis_rrf, opr& cur_commit_ptr){ /// size supposed to be equal to rrf
            opr& next_rrf = mis_rrf + 1;
            req_ptr <<= next_rrf;

            freenum <<= (RRF_ENTRIES - next_rrf) + cur_commit_ptr;
            zif(cur_commit_ptr >= next_rrf){
                ///// the free entry is bubble
                /// it must be >= because should be full only (freenum = 0)
                freenum <<= (cur_commit_ptr - next_rrf).uext(RRF_SEL + 1);
            }
        }

        /////// unfortunately rename and commit can be occur at the same time
        void do_rename_or_commit(){
            freenum <<= (freenum + commit_req_size - rename_req_size);
        }

        ////// on the table there should no conflict (rename<->wb<->commit)
        ////////////////it is ok to not have req1
        void on_rename(opr& req2){
            ////// is_renamable must be use
            rename_req_size = req2.uext(2) + 1;
            do_rename_or_commit();
            ////// rename have more priority than write back
            SET_ASM_PRI_TO_MANUAL(RRF_RENAME_PRI);  ///CTRL RRF

            table[req_ptr](rrf_valid) <<= 0;
            zif(req2){
                table[req_ptr+1](rrf_valid) <<= 0;
                ///// request 2 will not set if req1 is set
            }
            opr& next_rrf = req_ptr + rename_req_size;
            next_rrf_cycle <<= req_ptr > next_rrf;
            req_ptr <<= next_rrf;
            SET_ASM_PRI_TO_AUTO();  ///CTRL RRF

        }

        ////// write back and rename cannot disable the same valid register at the same time
        void on_wback(opr& wb_ptr, opr& wb_data){
            auto agent = table[wb_ptr];
            agent(rrf_valid) <<= 1;
            agent(rrf_data ) <<= wb_data;
        }

        ///// it
        std::pair<opr&, opr&> on_commit(opr& com_ptr, opr& com1Cond, opr& com2Cond){
            ///// com1Cond is only raw condition in the reorder buffer we must check that it is valid or not
            ///// com2Cond
            m_wire(com1Avail, 1);
            m_wire(com2Avail, 1);
            opr& round_over = (req_ptr < com_ptr) | (freenum == 0); //// no free turn around
            opr& rem = gr(round_over, req_ptr) - com_ptr; //// rem means remain to commit
            com1Avail = rem > 0; //// unequal to 0
            com2Avail = rem > 1; //// unequal to 1 and 0

            //////          resource in rrf(are there actually) & com_condition(finish)
            opr& res_com1 = com1Avail&com1Cond;
            opr& res_com2 = res_com1&com2Avail&com2Cond;
            commit_req_size = res_com1.uext(2) + res_com2.uext(2);
            do_rename_or_commit(); ////// rename and commit can occur at the same time
            return {res_com1, res_com2};
        }


    };

}

#endif //KATHRYN_EXAMPLE_O3_RRF_H
