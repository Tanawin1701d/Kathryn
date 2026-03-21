//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSV_H
#define KATHRYN_SRC_EXAMPLE_O3_RSV_H

#include "stage_struct.h"

namespace kathryn::o3{

    ////////////////////////////////////////////////
    ///  expected priority
    ///  |    g1    |    g2                      |          g3
    ///  |mispredict|write_entry > update sort bit| suc_pred/bypass/issue
    ///  g1, g2, g3 cannot happend at the same time
    ///  ////////////////////////////////////////////////

    static int RSV_MIS_PRED_PRIORITY         = DEFAULT_UE_PRI_USER + 3;
    static int RSV_WRITE_ENTRY_PRED_PRIORITY = DEFAULT_UE_PRI_USER + 2;
    static int RSV_SORTBIT_RST_PRED_PRIORITY = DEFAULT_UE_PRI_USER + 1;

    struct RsvBase{
        SlotMeta _meta;
        Table    _table;
        RegSlot  exec_src;
        SyncPip  sync {"rsv_exec_sync"}; ///CTRL RSV_SHARED

        ZyncSimProb*   issue_probe = nullptr;     ///DC
        TableSimProbe* station_probe = nullptr;   ///DC

        void set_sim_probe(ZyncSimProb* issue_p, TableSimProbe* station_p){  ///DC
            issue_probe = issue_p;                                         ///DC
            station_probe = station_p;                                     ///DC
            assert(station_probe != nullptr);                             ///DC
            station_probe->init(&_table);                                 ///DC
        }                                                                ///DC

        RsvBase(const SlotMeta& meta, int amt_row):
        _meta(meta),_table(meta, amt_row),
        exec_src(meta){
            //_table.make_col_reset_event(busy, 0);
            _table.make_reset_event(0);
            exec_src.make_reset_event();
        }

        virtual ~RsvBase() = default;

        virtual void build_issue(BroadCast& bc) = 0;

        Operable& slot_ready(WireSlot& iw){
            return iw(busy) && iw(rsValid_1) && iw(rsValid_2);
        }

        void try_ow_spec_bit(WireSlot& iw, BroadCast& bc){
            ///////// we have to override the spec bit if it is on the fly
            // auto& is_spec    = iw(spec);
            // auto& spec_tag_idx= iw(spec_tag);
            // //// send data
            //
            // zif ( is_spec && bc.check_is_suc(spec_tag_idx)){
            //     exec_src(spec) <<= 0;
            // }
            zif ( bc.check_is_suc(iw)){
                exec_src(spec) <<= 0;
            }
        }

        virtual void write_entry(opr& bin_idx, WireSlot& iw){
            SET_ASM_PRI_TO_MANUAL(RSV_WRITE_ENTRY_PRED_PRIORITY); ///CTRL RSV_SHARED
            _table[bin_idx] <<= iw;
            SET_ASM_PRI_TO_AUTO(); ///CTRL RSV_SHARED
        }

        virtual void on_issue(opr& issue_idx, WireSlot& iw){
            exec_src <<= iw;
            _table[issue_idx](busy) <<= 0;
        }
        virtual void on_issue(OH issue_oh_idx, WireSlot& iw){
            exec_src <<= iw;
            _table[issue_oh_idx](busy) <<= 0;
        }

        virtual void on_mis_pred(opr& fix_tag){

            SET_ASM_PRI_TO_MANUAL(RSV_MIS_PRED_PRIORITY); ///CTRL RSV_SHARED
            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){
                auto& is_busy    = lhs(busy);
                auto& is_spec    = lhs(spec);
                auto& my_spec_tag = lhs(spec_tag);
                //////// do bypass the system
                zif (is_busy & is_spec & ((my_spec_tag&fix_tag) != 0)){
                    is_busy <<= 0;
                }
            });
            SET_ASM_PRI_TO_AUTO(); ///CTRL RSV_SHARED


        }
        virtual void on_suc_pred(opr& suc_tag){
            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){
                auto& is_busy  = lhs(busy);
                auto& is_spec  = lhs(spec);
                auto& my_spec_tag= lhs(spec_tag);
                //////// do bypass the system
                zif (is_busy & is_spec & (my_spec_tag == suc_tag)){
                    is_spec <<= 0;
                }
            });
        }
        //// update the src register
        virtual void on_bypass(ByPass bp){
            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){
                for (int i = 1; i <= 2; i++){
                    auto& is_busy     = lhs(busy);
                    auto& is_rs_valid  = lhs(str(rsValid_) + to_s(i));
                    auto& phy_idx     = lhs(str(phyIdx_) + to_s(i));
                    //////// do bypass the system
                    zif (is_busy){
                        zif ((~is_rs_valid) & (phy_idx(0, RRF_SEL) == bp.rrf_idx)){
                            phy_idx <<= bp.val;
                            is_rs_valid <<= 1;
                        }
                    }
                }
            });
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSV_H
