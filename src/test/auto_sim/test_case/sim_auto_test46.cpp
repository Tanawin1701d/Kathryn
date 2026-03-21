// //
// // Created by tanawin on 27/3/2567.
// //
//
// #include "kathryn.h"
// #include "test/auto_sim/sim_auto_interface.h"
// #include "auto_test_interface.h"
//
//
//
// namespace kathryn{
//
//     class test_sim_mod46: public Module{
//     public:
//         m_reg(a, 8);
//         m_reg(b, 8);
//         m_reg(c, 8);
//         m_reg(d, 8);
//         m_reg(e, 8);
//         m_reg(f, 8);
//         m_wire(pip_i, 1);
//
//         explicit test_sim_mod46(int x): Module(){}
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
//                         pip_tran_when("decode", pip_i){a <<= a + 1;}
//                     }
//                     ///// decode pipe
//                     pip("decode"){
//                         seq{
//                             b <<= a;
//                             cif((a + 1) == 3){
//                                 sy_wait(3);
//                             }
//                         }
//                     }
//
//                     seq{
//                         sy_wait(3);
//                         pip_i = 1;
//
//                     }
//                 }
//
//             }
//         }
//
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test46.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test46.vcd";
//
//
//     class sim46 :public SimAutoInterface{
//     public:
//
//         test_sim_mod46* _md;
//
//         sim46(test_sim_mod46* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                                 _md(md){
//         }
//
//         void describe_con() override{
//
//             con_next_cycle(3);
//             test_and_print("test_pip_val: A", ull(_md->a), 0);
//             test_and_print("test_pip_val: B", ull(_md->b), 0);
//             con_next_cycle(2);
//             test_and_print("test_pip_val: A", ull(_md->a), 1);
//             test_and_print("test_pip_val: B", ull(_md->b), 0);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: A", ull(_md->a), 1);
//             test_and_print("test_pip_val: B", ull(_md->b), 1);
//
//         }
//
//
//
//     };
//
//
//     class Sim46TestEle: public AutoTestEle{
//     public:
//         explicit Sim46TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod46, 1);
//             start_model_kathryn();
//             sim46 simulator((test_sim_mod46*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim46TestEle ele46(46);
// }