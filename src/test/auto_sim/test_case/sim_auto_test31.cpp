//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

//
//
// namespace kathryn{
//
//     class test_sim_mod31: public Module{
//     public:
//         m_reg(a, 32);
//         m_reg(b, 32);
//         m_reg(c, 32);
//         m_reg(d, 32);
//         m_wire(is, 1);
//         m_wire(freez , 1);
//         m_wire(freez2, 1);
//
//         explicit test_sim_mod31(int x): Module(){}
//
//         void flow() override{
//
//             // pip_wrap{
//             //     /////// pipe block 0
//             //     pip_blk{
//             //         intr_start(is);
//             //         intr_reset(is);
//             //
//             //         cif(a < 5){
//             //             a = a + 1;
//             //         }celse{
//             //             cwhile(true){
//             //                 seq {
//             //                     freez  = 1;
//             //                 }
//             //             }
//             //         }
//             //     }
//             //     /////// pipe block 1
//             //     pip_blk{
//             //         b = b + 1;
//             //     }
//             // }
//
//
//             seq{
//                 sy_wait(10);
//                 par{
//                     is = 1;
//                     a  = 0;
//                 }
//             }
//         }
//
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test31.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test31.vcd";
//
//
//     class sim31 :public SimAutoInterface{
//     public:
//
//         test_sim_mod31* _md;
//
//         sim31(test_sim_mod31* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                                 _md(md)
//         {}
//
//         void describe_con() override{
//
// // //            /*** start cycle*/
// //               for (int i = 1; i <= 5; i++){
// //                   test_and_print("test_pip_val before reset_and_restart and get_stuck", ull(_md->a), i);
// //                   con_end_cycle();
// //                   test_and_print("test_pip_val before freez", ull(_md->freez), 0);
// //                   con_next_cycle(1);
// //               }
// //               for (int i = 0; i < 6; i++){
// //                   con_end_cycle();
// //                   test_and_print("test_pip_val freez", ull(_md->freez), 1);
// //                   con_next_cycle(1);
// //               }
// //
// //             for (int i = 1; i <= 5; i++){
// //                 test_and_print("test_pip_val after reset_and_restart: a", ull(_md->a), i);
// //                 con_end_cycle();
// //                 test_and_print("test_pip_val after freez", ull(_md->freez), 0);
// //                 con_next_cycle(1);
// //             }
//
//         }
//
//
//
//     };
//
//
//     class Sim31TestEle: public AutoTestEle{
//     public:
//         explicit Sim31TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod31, 1);
//             start_model_kathryn();
//             sim31 simulator((test_sim_mod31*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim31TestEle ele31(31);
// }