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
//     class test_sim_mod23: public Module{
//     public:
//         m_reg(a, 8);
//         m_reg(b, 8);
//         m_reg(c, 8);
//         m_reg(d, 8);
//         m_reg(e, 8);
//         m_reg(f, 8);
//
//         explicit test_sim_mod23(int x): Module(){}
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
//                         pip_tran("decode"){
//                             ////arb_req("spec_tag_gen");
//                             a <<= a + 1;
//                         }
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
//                 }
//
//             }
//         }
//
//
//         /**
//          *      --------- req ----->
//          *      <-------- offer ----
//          *      --------- occ (when all related signal is done) ----->
//          * **/
//
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test23.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test23.vcd";
//
//
//     class sim23 :public SimAutoInterface{
//     public:
//
//         test_sim_mod23* _md;
//
//         sim23(test_sim_mod23* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                                 _md(md)
//         {}
//
//         void describe_con() override{
//
//             con_next_cycle(2);
//             test_and_print("test_pip_val: A", ull(_md->a), 1);
//             test_and_print("test_pip_val: B", ull(_md->b), 0);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: A", ull(_md->a), 2);
//             test_and_print("test_pip_val: B", ull(_md->b), 1);
//             con_next_cycle(4);
//             test_and_print("test_pip_val: A", ull(_md->a), 3);
//             test_and_print("test_pip_val: B", ull(_md->b), 2);
//             con_next_cycle(1);
//             test_and_print("test_pip_val: A", ull(_md->a), 4);
//             test_and_print("test_pip_val: B", ull(_md->b), 3);
//
//         }
//
//
//
//     };
//
//
//     class Sim23TestEle: public AutoTestEle{
//     public:
//         explicit Sim23TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod23, 1);
//             start_model_kathryn();
//             sim23 simulator((test_sim_mod23*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim23TestEle ele23(23);
// }