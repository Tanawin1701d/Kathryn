//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ORSV_H
#define KATHRYN_SRC_EXAMPLE_O3_ORSV_H

#include "rsv.h"

namespace kathryn::o3{

        struct ORsv: RsvBase{
        const int  RSV_IDX = 0;
        RegArch&   reg_arch;
        bool       sort_req = false;
        m_wire(check_idx, _table.get_sufficient_idx_size(true));
        m_wire(dbg_isSlotReady, 1); ///DC

        ORsv(int rsv_idx, SlotMeta meta,
             int amt_row , RegArch& reg_arch,
             const SlotMeta& osm = sm_rsv_o):
            RsvBase(osm + meta, amt_row),
            RSV_IDX(rsv_idx),
            reg_arch(reg_arch),
            sort_req(osm.is_there_field(sort_bit)){}

        void reset_sort_bit(){
            SET_ASM_PRI_TO_MANUAL(RSV_SORTBIT_RST_PRED_PRIORITY); ///CTRL RSV_SHARED
            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){
                lhs(sort_bit) <<= lhs(sort_bit) & (~reg_arch.rrf.next_rrf_cycle);
            });
            SET_ASM_PRI_TO_AUTO(); ///CTRL RSV_SHARED
        }


        void try_write_entry(opr& target_idx_ptr, opr& bin_idx, WireSlot& iw){
            zif (target_idx_ptr == RSV_IDX){
                RsvBase::write_entry(bin_idx, iw);
            }
        }

            ////// friend table is used incase of two table join the same index
        pair<opr&, opr&> build_free_index(opr* except_idx, RsvBase* friend_rsv = nullptr){


            Table eff_table = _table; //// = will not build new hardware
            if (friend_rsv != nullptr){
                eff_table = (eff_table.join_table_by_row_interleave(friend_rsv->_table));
            }

            auto [iw, bin_idx] = eff_table.do_reduc_bin_idx([&](
             WireSlot& lhs, Operable* lidx,
             WireSlot& rhs, Operable* ridx) -> opr&{
                if (except_idx == nullptr){
                    return ~lhs(busy); //// we don't care rhs
                }
                return ~lhs(busy) && ((*lidx) != (*except_idx));
            });
            return {iw(busy), bin_idx};
        }

        

        void build_issue(BroadCast& bc) override{
            /*
            * find the free slot
            */
            auto [iw, oh_idx] = _table.do_reduc_oh_idx(
                [&](WireSlot& lhs, Operable* lidx,
                    WireSlot& rhs, Operable* ridx)-> Operable&{
                    lhs.try_add_wire(entry_ready, slot_ready(lhs));
                    rhs.try_add_wire(entry_ready, slot_ready(rhs));

                    if (sort_req){
                        auto& ready_eq   = lhs(entry_ready) == rhs(entry_ready);
                        auto& sort_bit_eq = lhs(sort_bit) == rhs(sort_bit);
                        return
                            (lhs(entry_ready) && (~rhs(entry_ready))) ||
                            (ready_eq && (lhs(sort_bit) < rhs(sort_bit))) ||
                            (ready_eq &&  sort_bit_eq && (lhs(rrftag) < rhs(rrftag)));
                    }
                    ////// no sort request
                    return lhs(entry_ready);
                }
            );

            check_idx = oh_idx.get_idx_ptr();
            /**
             * issue sync
             */
            dbg_isSlotReady = slot_ready(iw); ///DC

            if (sort_req){
                reset_sort_bit();
            }

            cwhile(true){ ///CTRL RSV_SHARED
                zyncc(sync, dbg_isSlotReady){ try_init_probe(issue_probe); ///CTRL RSV_SHARED
                    //////// reset the table
                    on_issue(oh_idx, iw);
                    try_ow_spec_bit(iw, bc);
                }
            }
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ORSV_H
