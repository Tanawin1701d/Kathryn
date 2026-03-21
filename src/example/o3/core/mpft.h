//
// Created by tanawin on 27/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_MPFT_H
#define KATHRYN_SRC_EXAMPLE_O3_MPFT_H

#include "slot_param.h"
#include "example/o3/simulation/prober_grp.h"  ///DC

namespace kathryn::o3{

    //// miss predict fix table
    struct Mpft{
        Table _table;

        Mpft(): _table(sm_mpft, SPECTAG_LEN){
            _table.make_col_reset_event(mpft_valid, 0);
            _table.make_col_reset_event(mpft_fixTag, 0);
            /////// debug probe
            data_struct_prob_grp.mpft.init(&_table); ///DC
        }

        void on_pred_suc(opr& tag1){

            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){
                lhs(mpft_fixTag) <<= (lhs(mpft_fixTag) & (~tag1));
                zif (tag1.sl(row_idx)){ ///// matched tag
                    ///// clean entire row
                    lhs(mpft_valid) <<= 0;
                    lhs(mpft_fixTag) <<= 0;
                }
            });

        }
        void on_mis_pred(){
            _table.do_glob_col_asm({mpft_valid}, 0);
            _table.do_glob_col_asm({mpft_fixTag}, 0);
        }
        void on_add_new(opr& set_tag1, opr& tag1,
                      opr& set_tag2, opr& tag2){

            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){

                opr& is_tag1 = tag1.sl(row_idx);
                opr& is_tag2 = tag2.sl(row_idx);
                Reg& vl     = lhs(mpft_valid);
                Reg& ft     = lhs(mpft_fixTag);
                ////// set the new row
                zif (is_tag1 && set_tag1){
                    vl <<= 1;
                    ft <<= tag1;
                    zif (set_tag2) ft <<= (tag1 | tag2);
                }
                zif (is_tag2 && set_tag2){
                    vl <<= 1;
                    ft <<= tag2;
                }
                ///// it is suppose to be other tag because vl for both is not set
                zif(vl){
                    zif(set_tag1) ft <<= (ft | tag1);
                    zif(set_tag2) ft <<= (ft | tag2);
                    zif(set_tag1 && set_tag2) ft <<= (ft | tag1 | tag2);
                }
            });
        }

        opr& get_fix_tag(OH search_tag){
            return _table[OH(search_tag)](mpft_fixTag).v();
        }

        opr& is_used(int spec_idx){return _table(spec_idx)(mpft_valid);}


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_MPFT_H
