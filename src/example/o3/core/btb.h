// // DISABLE THIS FILE
// // Created by tanawin on 13/12/25.
// //
//
// #ifndef EXAMPLE_O3_CORE_BTB_H
// #define EXAMPLE_O3_CORE_BTB_H
//
// #include "parameter.h"
// #include "slot_param.h"
//
// namespace kathryn::o3{
//
//     struct Btb{
//         Table valid_table{ sm_btb, 8}; /// 8*64 = 512    /// todo change tot
//         //m_mem(valid, BTB_IDX_NUM, 1); ///// it is supposed to be register
//         m_mem(bia  , BTB_IDX_NUM, ADDR_LEN);
//         m_mem(bta  , BTB_IDX_NUM, ADDR_LEN);
//
//         Btb(){
//             valid_table.make_reset_event(0, CM_NEGEDGE);
//         }
//
//         void on_commit(opr& src_addr, opr& target_addr){
//             opr& btb_addr  = src_addr.sl(BTB_IDX_SEL_START, BTB_IDX_SEL_STOP);
//             opr& btb_addr_l = btb_addr.sl(0, 6);
//             opr& btb_addr_h = btb_addr.sl(6, 9);
//             m_val(one, 64, 1);
//             opr& commited_bit = one << btb_addr_l;
//             SET_CLK_MODE2NEG_EDGE();
//                 valid_table.do_cus_logic([&](RegSlot& reg_slot, int row_idx){
//                     zif ( btb_addr_h == row_idx){
//                         reg_slot(0) <<= ( reg_slot(0) | commited_bit );
//                     }
//                 });
//                 bia  [btb_addr] <<= src_addr;
//                 bta  [btb_addr] <<= target_addr;
//             SET_CLK_MODE2DEF();
//         }
//
//         ///// hit and address
//         std::pair<opr&, opr&> on_inquire(opr& cur_pc, opr& inv2){
//             /////// pre declaration
//             opr& btb_addr  = cur_pc.sl(BTB_IDX_SEL_START, BTB_IDX_SEL_STOP);
//             opr& btb_addr_l = btb_addr.sl(0, 6);
//             opr& btb_addr_h = btb_addr.sl(6, 9);
//             opr& row_valid = (valid_table[btb_addr_h](0).v() >> btb_addr_l).sl(0);
//             /////// retrieve the data from the memory
//             m_reg(tag_data   , ADDR_LEN);
//             m_reg(target_addr, ADDR_LEN);
//             SET_CLK_MODE2NEG_EDGE();
//             tag_data    <<= bia[btb_addr];
//             target_addr <<= bta[btb_addr];
//             SET_CLK_MODE2DEF();
//             /////// check it is hit or not
//             opr& hit1 = (tag_data == cur_pc) && row_valid;
//             opr& hit2 = (tag_data == cur_pc) && row_valid &&
//                         (~inv2);
//
//             return {hit1 | hit2, target_addr};
//         }
//     };
//
// }
//
// #endif //KATHRYN_BTB_H