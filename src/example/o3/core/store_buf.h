//
// Created by tanawin on 9/12/25.
//

#ifndef EXAMPLE_O3_CORE_STOREBUF_H
#define EXAMPLE_O3_CORE_STOREBUF_H

#include "search_idx.h"
#include "slot_param.h"

namespace kathryn::o3{

    struct StoreBuf{
        ///// main data
        LdStStage& lss;
        BroadCast& bc;
        Table _table{sm_store_buf, STBUF_ENT_NUM};
        m_mem(daytas , STBUF_ENT_NUM, DATA_LEN);
        ///// meta data
        m_reg(fin_ptr, STBUF_ENT_SEL); ////
        m_reg(com_ptr, STBUF_ENT_SEL); //// complete_ptr
        m_reg(ret_ptr, STBUF_ENT_SEL); //// retire_ptr
        ///// search result of zero bit
        SearchResult nb1, ne1, nb0;
        m_wire(full_next, 1);  /// full after kill mispredict
        m_wire(empty_next, 1); /// empty after kill mispredict



        StoreBuf(LdStStage& lss, BroadCast& bc):
        lss(lss),
        bc(bc),
        nb1(search_idx(_table, 1, true , bc, true )),
        ne1(search_idx(_table, 1, false, bc, true )),
        nb0(search_idx(_table, 0, true , bc, true )){
            fin_ptr.make_reset_event();
            com_ptr.make_reset_event();
            ret_ptr.make_reset_event();
            _table.make_col_reset_event(busy);
            _table.make_col_reset_event(complete);
            _table.make_col_reset_event(spec);

        }

        void on_commit(){
            com_ptr <<= com_ptr + 1;
            _table[com_ptr](complete) <<= 1;
        }

        void on_new_entry(RegSlot& src, opr& dayta, opr& mem_addr){

            //// it may be overrided with on_mis_pred
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
            _table[fin_ptr](complete) <<= 0;
            _table[fin_ptr][mem_addr] <<= mem_addr;
            _table[fin_ptr]           <<= src; /// busy, spec, spec_tag
            daytas[fin_ptr]           <<= dayta;
            fin_ptr                   <<= (fin_ptr + 1);
            zif(bc.check_is_suc(src)){
                _table[fin_ptr](spec) <<= 0;
            }
            SET_ASM_PRI_TO_AUTO();
        }

        void on_mis_pred(opr& fix_tag){
            ////// destroy the internal buffer
            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){
                //////// do bypass the system
                zif ( lhs(spec) &
                    ((lhs(spec_tag)&fix_tag) != 0)){
                    lhs(busy) <<= 0;
                }
            });
            ////// refit meta data
            /// if empty or full the value of fin is equal to old system
            zif (~empty_next && ~full_next){
                fin_ptr <<= ne1.s_idx + 1; //// pre declare that there is no bubble
                ///// there at least 1 and 1 begin detection and 1 end detection must have it
                zif( (nb1.s_idx == 0) && (ne1.s_idx == (_table.get_num_row()-1)) ){
                    fin_ptr <<= nb0.s_idx;
                }
            }

            zif(empty_next){ ///// the system is empty next
                com_ptr <<= fin_ptr;
                ret_ptr <<= fin_ptr;
            }
        }

        void on_suc_pred(opr& suc_tag){
            _table.do_cus_logic([&](RegSlot& lhs, int row_idx){
                //////// do bypass the system
                zif (lhs(spec) &
                    (lhs(spec_tag) == suc_tag)){
                    lhs(spec) <<= 0;
                }
            });
        }

        opr& is_full(){return (fin_ptr == ret_ptr) && (_table[fin_ptr](busy).v());}



        std::pair<opr&, opr&> search_newest(opr& addr){

            /// find newst system.
             auto[result, bin_idx] = _table.findMBO_BIDX(true, fin_ptr,
                 [&](RegSlot& lhs)->opr&{
                 return lhs(busy) & (lhs(mem_addr) == addr);
             });
            return {result(busy) & (result(mem_addr) == addr), daytas[bin_idx]};
        }

        void flow(){ //// retire and do other thing
            full_next  = ~(nb0.s_valid);
            empty_next = ~(ne1.s_valid);

            ///// retire system
            WireSlot retire_slot(_table[ret_ptr].v(), "retire_slot");
            lss.dmem_rwaddr = retire_slot(mem_addr);
            lss.dmem_wdata = daytas[ret_ptr];

            zif (retire_slot(busy) & retire_slot(complete) & (~bc.is_br_miss_pred())){
                zif(lss.dmem_we = 1){ ///// try to make it equal to one if not update will not occur because memory hold them all  ///CTRL GROB
                    ret_ptr <<= (ret_ptr + 1);
                    _table[ret_ptr](busy)     <<= 0;
                    _table[ret_ptr](complete) <<= 0;
                }
            }



        }



    };

}

#endif //EXAMPLE_O3_CORE_STOREBUF_H