// //
// // Created by tanawin on 23/3/2025.
// //
//
//
// #include "kathryn.h"
// #include "test/auto_sim/sim_auto_interface.h"
// #include "auto_test_interface.h"
//
//
//
// namespace kathryn{
//
//     class test_sim_mod48: public Module{
//     public:
//         m_reg(a, 8);
//         m_reg(b, 8);
//         m_reg(c, 8);
//         m_reg(d, 8);
//         m_reg(e, 8);
//         m_reg(f, 8);
//
//
//         explicit test_sim_mod48(int x): Module(){}
//
//         void flow() override{
//
//             seq{
//                 par{
//                     a = 5; b = 0;
//                     c = 0; d = 0;
//                 }
//                 par{
//                     ///// fetch pipe
//                     pip("fetch"){ auto_start
//                         pip_tran("decode"){
//                             b <<= a;
//                             a <<= a + 1;
//                         }
//                     }
//                     ///// decode pipe
//                     pip("decode"){
//                         pip_m_tran{
//                             zif(b(0) == 0){ c <<= b; }
//                             zif(b(0) == 1){ d <<= b; }
//                             tran_to_wc("exec0", b(0) == 0);
//                             tran_to_wc("exec1", b(0) == 1);
//
//                         }
//                     }
//
//                     pip("exec0"){e <<= c;}
//
//                     pip("exec1"){f <<= d;}
//
//                 }
//
//             }
//         }
//
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test48.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test48.vcd";
//
//
//     class sim48 :public SimAutoInterface{
//     public:
//
//         test_sim_mod48* _md;
//
//         sim48(test_sim_mod48* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                                 _md(md)
//         {}
//
//         void describe_con() override{
//             std::cout << TC_BLUE << "test pipe multipath" << TC_DEF << std::endl;
//             con_next_cycle(2);
//             test_and_print("test_pip_val: B", ull(_md->b), 5);
//             test_and_print("test_pip_val: B", ull(_md->c), 0);
//             test_and_print("test_pip_val: B", ull(_md->d), 0);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 6);
//             test_and_print("test_pip_val: B", ull(_md->c), 0);
//             test_and_print("test_pip_val: B", ull(_md->d), 5);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 7);
//             test_and_print("test_pip_val: B", ull(_md->c), 6);
//             test_and_print("test_pip_val: B", ull(_md->d), 5);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 8);
//             test_and_print("test_pip_val: B", ull(_md->c), 6);
//             test_and_print("test_pip_val: B", ull(_md->d), 7);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 9);
//             test_and_print("test_pip_val: B", ull(_md->c), 8);
//             test_and_print("test_pip_val: B", ull(_md->d), 7);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 10);
//             test_and_print("test_pip_val: B", ull(_md->c), 8);
//             test_and_print("test_pip_val: B", ull(_md->d), 9);
//
//         }
//
//
//
//     };
//
//
//     class Sim48TestEle: public AutoTestEle{
//     public:
//         explicit Sim48TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod48, 1);
//             start_model_kathryn();
//             sim48 simulator((test_sim_mod48*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim48TestEle ele48(48);
// }