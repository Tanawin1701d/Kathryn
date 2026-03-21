//
// Created by tanawin on 26/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
#define KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H

#include "broad_cast.h"
#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    struct TagGen{
        BroadCast& bc;
        m_reg(brdepth , BRDEPTH_LEN);
        m_reg(tagreg  , SPECTAG_LEN);
        m_wire(sp_tag1_result, SPECTAG_LEN);
        m_wire(sp_tag2_result, SPECTAG_LEN);

        explicit TagGen(BroadCast& bc):
        bc(bc){
            brdepth.make_reset_event();
            tagreg.make_reset_event(1);
        }

        void on_mis_pred(opr& mis_tag){
            brdepth <<= 0;
            tagreg  <<= gr(mis_tag.sl(0), mis_tag.sl(1, SPECTAG_LEN));
            ///// the tag reg supposed to be the last tag that valid and shift right 1
        }

        void on_suc_pred(){
            brdepth <<=  (brdepth - 1);
        }

        opr& is_all_genble(opr& branch_valid1, opr& branch_valid2){

            opr& amt_gen = (branch_valid1.uext(2) +
                           branch_valid2.uext(2)).uext(BRDEPTH_LEN);
            opr& amt_free = bc.is_br_succ_pred().uext(BRDEPTH_LEN);

            return (brdepth + amt_gen) <=
            (m_opr_val("ent_num", BRDEPTH_LEN, BRANCH_ENT_NUM) + amt_free);
        }

        opr& round_shift1(Reg& src){
            return g(src(0, SPECTAG_LEN-1), src(SPECTAG_LEN-1));
        }

        opr& round_shift2(Reg& src){
            return g(src(0, SPECTAG_LEN-2), src(SPECTAG_LEN-2, SPECTAG_LEN));
        }

        /////// is_all_genble should be used first
        std::pair<opr&, opr&> allocate(
            opr& branch_valid1,Reg& spec1,
            opr& branch_valid2,Reg& spec2){
            spec1 <<= (brdepth != 0);
            sp_tag1_result = mux(branch_valid1,
                                    round_shift1(tagreg),
                                    tagreg);
            spec2 <<= (brdepth != 0) || (branch_valid1);
            sp_tag2_result = mux(branch_valid1 & branch_valid2, round_shift2(tagreg),
                            mux(branch_valid2, round_shift1(tagreg), ///// branch_valid1 is false
                                sp_tag1_result
                            ));
            tagreg <<= sp_tag2_result;
            brdepth <<= ((((brdepth + branch_valid1)
                                    + branch_valid2)
                                    - bc.is_br_succ_pred()));
            return {sp_tag1_result, sp_tag2_result};
        }

            // ///// allocate branch 1
            // zif (branch_valid1){
            //     spec1        <<= (brdepth != 0);
            //     sp_tag1_result   = round_shift1(tagreg);
            //     tagreg       <<= sp_tag1_result;
            // }
            // ///// allocate branch 2
            // spec2 <<= (brdepth != 0) || (branch_valid1);
            // zif (branch_valid2){
            //     zif (branch_valid1){
            //         sp_tag2_result = round_shift2(tagreg);
            //     }
            //     zelse{
            //         sp_tag2_result = round_shift1(tagreg);
            //     }
            //     tagreg <<= sp_tag2_result;
            // }
            //// update internal structure


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_TAGGEN_H
