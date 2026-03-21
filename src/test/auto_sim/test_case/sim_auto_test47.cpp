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
//     class test_sim_mod47: public Module{
//     public:
//         m_reg(a, 8);
//         m_reg(b, 8);
//         m_reg(c, 8);
//         m_reg(d, 8);
//         m_reg(e, 8);
//         m_reg(f, 8);
//         m_wire(int_rst, 1);
//         m_wire(int_sta, 1);
//
//         explicit test_sim_mod47(int x): Module(){}
//
//         void flow() override{
//
//             seq{
//                 par{
//                     a = 0; b = 0;
//                     c = 0; d = 0;
//                 }
//                 par{
//                     ///// fetch pipe
//                     pip("fetch"){ auto_start
//                         pip_tran("decode"){a <<= a + 1;}
//                     }
//                     ///// decode pipe
//                     pip("decode"){ intr_reset(int_rst); intr_start(int_sta);
//                         pip_tran("exec"){b <<= a;}
//                     }
//
//                     pip("exec"){
//                         c <<= b;
//                     }
//
//                     seq{
//                         sy_wait(4);
//                         int_rst = 1;
//                         sy_wait(4);
//                         int_sta = 1;
//
//                     }
//                 }
//
//             }
//         }
//
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test47.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test47.vcd";
//
//
//     class sim47 :public SimAutoInterface{
//     public:
//
//         test_sim_mod47* _md;
//
//         sim47(test_sim_mod47* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                                 _md(md)
//         {}
//
//         void describe_con() override{
//             std::cout << TC_BLUE << "test pipe with interupt reset" << TC_DEF << std::endl;
//             con_next_cycle(2);
//             test_and_print("test_pip_val: B", ull(_md->b), 0);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 1);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 2);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B", ull(_md->b), 3);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: B hold", ull(_md->b), 3);
//             con_next_cycle(6);
//             test_and_print("test_pip_val: B renew", ull(_md->b), 5);
//
//         }
//
//
//
//     };
//
//
//     class Sim47TestEle: public AutoTestEle{
//     public:
//         explicit Sim47TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod47, 1);
//             start_model_kathryn();
//             sim47 simulator((test_sim_mod47*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim47TestEle ele47(47);
// }