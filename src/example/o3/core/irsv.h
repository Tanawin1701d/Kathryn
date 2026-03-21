//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IRSV_H
#define KATHRYN_SRC_EXAMPLE_O3_IRSV_H

#include "rsv.h"
#include "search_idx.h"


namespace kathryn::o3{



    struct IRsv: RsvBase{

        m_wire(check_idx, _table.get_sufficient_idx_size(false));
        const int  RSV_IDX = 0;
        Reg& alloc_ptr;
        SearchResult b1 ,  e1,  e0;
        SearchResult nb1, ne1, nb0; /// search to fix alloc pointer

        m_wire(dbg_b1_valid, 1);    ///DC
        m_wire(dbg_e1_valid, 1);    ///DC
        m_wire(dbg_e0_valid, 1);    ///DC
        m_wire(dbg_nb1_valid, 1);   ///DC
        m_wire(dbg_ne1_valid, 1);   ///DC
        m_wire(dbg_nb0_valid, 1);   ///DC

        m_wire(dbg_b1_idx, 3);    ///DC
        m_wire(dbg_e1_idx, 3);    ///DC
        m_wire(dbg_e0_idx, 3);    ///DC
        m_wire(dbg_nb1_idx, 3);    ///DC
        m_wire(dbg_ne1_idx, 3);    ///DC
        m_wire(dbg_nb0_idx, 3);    ///DC

        IRsv(int rsv_idx  , SlotMeta meta,
             int index_size, std::string debug_name,
             BroadCast& bc):
            RsvBase(sm_rsv_i + meta, 1 << index_size),
            RSV_IDX(rsv_idx),
            alloc_ptr(m_opr_reg("allocPtr_" + debug_name, index_size)),
            b1 (search_idx(_table, 1, true , bc, false)),
            e1 (search_idx(_table, 1, false, bc, false)),
            e0 (search_idx(_table, 0, false, bc, false)),
            nb1(search_idx(_table, 1, true , bc, true )),
            ne1(search_idx(_table, 1, false, bc, true )),
            nb0(search_idx(_table, 0, true , bc, true )){
            alloc_ptr.make_reset_event();
        }


        void try_write_entry(opr& target_idx_ptr, opr& bin_idx, WireSlot& iw){
            zif (target_idx_ptr == RSV_IDX){
                alloc_ptr <<= (bin_idx + 1);
                RsvBase::write_entry(bin_idx, iw);
            }
        }

        void on_mis_pred(opr& fix_tag) override{

            RsvBase::on_mis_pred(fix_tag);
            zif (nb0.s_valid){ /// there is empty space for next update
                /// if there is no 1 for next cycle  (the )
                zif(~nb1.s_valid){
                    alloc_ptr <<= 1; ////// it is empty
                }zelif((nb1.s_idx == 0) && (ne1.s_idx == (_table.get_num_row()-1))){
                    ////// case 0  is bubble there is 1 atstart and 1 at the end
                    alloc_ptr <<= nb0.s_idx; ///// nb0
                }zelse{
                    alloc_ptr <<= (ne1.s_idx+1);
                }
            }
            /// incase update when entrance the filler at rename stage will handle it
        }

        /**
         * ISSUE
         */

        pair<opr&, opr&> build_free_index(opr* req_idx, RsvBase* friend_rsv = nullptr){
            assert(friend_rsv == nullptr); ///DC

            opr* sel_idx = (req_idx == nullptr)? &alloc_ptr : req_idx;
            return {_table[*sel_idx](busy).v(), *sel_idx};
        }

        void build_issue(BroadCast& bc) override{

            /*
             *  the required Idx
             */
            check_idx = alloc_ptr;
            zif(e0.s_valid){ ///  there is empty slot
                check_idx = b1.s_idx;
                zif((b1.s_idx == 0) && (e1.s_idx == (_table.get_num_row()-1))){ ///// zero bubble
                    //// it may be entirely zeros but we will check it at slot_ready(iw)
                    check_idx = (e0.s_idx + 1);
                }
            }

            /**
             * the issue block
             */
            WireSlot iw(_table[check_idx].v());

            cwhile(true){                                             ///CTRL RSV_SHARED
                zyncc(sync, slot_ready(iw)){ try_init_probe(issue_probe); ///CTRL RSV_SHARED
                    //////// reset the table
                    on_issue(check_idx, iw); //// reset busy
                    try_ow_spec_bit(iw, bc);
                }
            }

            dbg_b1_valid     = b1.first;   ///DC
            dbg_e1_valid     = e1.first;   ///DC
            dbg_e0_valid     = e0.first;   ///DC
            dbg_nb1_valid     = nb1.first; ///DC
            dbg_ne1_valid     = ne1.first; ///DC
            dbg_nb0_valid     = nb0.first; ///DC

            dbg_b1_idx    =    b1.second;    ///DC
            dbg_e1_idx    =    e1.second;    ///DC
            dbg_e0_idx    =    e0.second;    ///DC
            dbg_nb1_idx    =    nb1.second;  ///DC
            dbg_ne1_idx    =    ne1.second;  ///DC
            dbg_nb0_idx    =    nb0.second;  ///DC
        }

    };


}

#endif //KATHRYN_SRC_EXAMPLE_O3_IRSV_H
