// //
// // Created by tanawin on 13/12/25.
// //
//
// #ifndef EXAMPLE_O3_CORE_GSHARE_H
// #define EXAMPLE_O3_CORE_GSHARE_H
//
// #include "kathryn.h"
// #include "mpft.h"
// #include "parameter.h"
//
//
// namespace kathryn::o3{
//
//     struct Pht{ ///// pattern history table
//         m_mem(pht0, GSH_PHT_NUM, 2); ///// for fetch
//         m_mem(pht1, GSH_PHT_NUM, 2); ///// for rob
//
//         Reg& read(opr& addr, bool first){
//             ///// target resource
//             MemBlock& target_block = first ? pht0 : pht1;
//             m_reg(read_result, 2);
//             ///// read the register
//             SET_CLK_MODE2NEG_EDGE();
//             read_result <<= target_block[addr];
//             SET_CLK_MODE2DEF();
//
//             return read_result;
//         }
//
//         void write(opr& addr, opr& data){
//             SET_CLK_MODE2NEG_EDGE();
//             pht0[addr] <<= data;
//             pht1[addr] <<= data;
//             SET_CLK_MODE2DEF();
//         }
//     };
//
//
//     struct Gshare{
//         Mpft&   mpft;
//         Pht     phts;
//         RegSlot bhrs{sm_bhrs};
//
//         m_reg (bhr_master, GSH_BHR_LEN);
//         m_wire(fet_pht_val, 2); ///// from fetch the data system
//         m_wire(rob_pht_val, 2); ///// from rob fetch
//
//         Gshare(Mpft& mpft): mpft(mpft){
//             bhrs     .make_reset_event(0);
//             bhr_master.make_reset_event(0);
//
//         }
//
//         void build_pht_reader(opr& fet_addr,
//                             opr& rob_addr){
//             fet_pht_val = phts.read(fet_addr, true ); ///// fetch got first bank
//             rob_pht_val = phts.read(rob_addr, false); ///// rob got second bank
//         }
//
//         void onSucPred_bhrUpdate(opr& suc_tag,
//                                  opr& fetch_hit_and_take){
//             bhrs[OH(suc_tag)] <<= gr(bhr_master(0, GSH_BHR_LEN - 1), fetch_hit_and_take);
//         }
//
//         void onMisPred_bhrUpdate(opr& mis_tag){
//
//             opr& fix_bhr = bhrs[OH(mis_tag)].v();
//             for (int idx = 0; idx < SPECTAG_LEN; idx++){
//                 bhrs(idx) <<= fix_bhr;
//             }
//             bhr_master <<= fix_bhr;
//
//         }
//
//         ///// it will invoke when btb is hit
//         void onCommit_bhrUpdate(opr& fetch_btb_hit,
//                                 opr& fetch_pred_cond){
//             /////// bhrs
//             zif (fetch_btb_hit){
//                 for (int idx = 0; idx < SPECTAG_LEN; idx++){
//                     bhrs(idx) <<= gr(bhrs(idx).sl(0, GSH_BHR_LEN - 1),  fetch_pred_cond);
//                 }
//                 bhr_master <<= gr(bhr_master(0, GSH_BHR_LEN - 1), fetch_pred_cond);
//             }
//         }
//
//         void onCommit_PhtUpdate(opr& commit_pht_addr,
//                                 opr& commit_cond){
//
//             /////// calculate new value and put it back
//             m_val(PHTENEG, 2, 0b00);
//             m_val(PHTEPOS, 2, 0b11);
//             /////// for 1 +1 /  for 0 -1  to operate the saved state machine
//             opr& rob_pht_val_op_val = mux(commit_cond, commit_cond.uext(2), PHTENEG);
//
//             opr& rob_pht_val_new = mux( (rob_pht_val == PHTENEG) && (~commit_cond), PHTENEG,
//                                 mux((rob_pht_val == PHTEPOS) && ( commit_cond), PHTEPOS,
//                                 rob_pht_val + rob_pht_val_op_val
//                                 ));
//             phts.write(commit_pht_addr, rob_pht_val_new);
//         }
//
//     };
//
// }
//
// #endif //EXAMPLE_O3_CORE_GSHARE_H