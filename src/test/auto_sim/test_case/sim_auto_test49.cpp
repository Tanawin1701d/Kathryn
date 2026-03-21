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
//     class test_sim_mod49: public Module{
//     public:
//         m_reg (a  , 8);
//         m_reg (b  , 8);
//         m_reg (c  , 8);
//         m_reg (d  , 8);
//         m_wire(x  , 1);
//         m_wire(st0, 1);
//         m_wire(st1, 1);
//         m_wire(st2, 1);
//
//
//         explicit test_sim_mod49(int x): Module(){}
//
//         void flow() override{
//
//             cwhile(true){
//                 offer("test"){
//                     ofc("a")    { a <<=  a + 1;}
//                     ofcc("b", x){ b <<=  b + 1;}
//                     ofc("c")    { c <<=  c + 1;}
//                 }
//             }
//
//
//             seq{
//                 sy_wait(5);
//                 par{
//                     zif(get_offer("test", "a")){
//                         st0 = 1;
//                         ack_offer("test", "a", st0);
//                     }
//                 }
//                 sy_wait(5);
//                 par{
//                     x = 1;
//                     zif(get_offer("test", "b")){
//                         st1 = 1;
//                         ack_offer("test", "b", st1);
//                     }
//
//                 }
//                 sy_wait(5);
//                 par{
//                     x = 1;
//                     zif(get_offer("test", "b")){
//                         st1 = 1;
//                         ack_offer("test", "b", st1);
//                     }
//                     zif(get_offer("test", "c")){
//                         st2 = 1;
//                         ack_offer("test", "c", st2);
//                     }
//                 }
//
//             }
//         }
//
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test49.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test49.vcd";
//
//
//     class sim49 :public SimAutoInterface{
//     public:
//
//         test_sim_mod49* _md;
//
//         sim49(test_sim_mod49* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                                 _md(md)
//         {}
//
//         void describe_con() override{
//             std::cout << TC_BLUE << "test offer" << TC_DEF << std::endl;
//             con_next_cycle(6);
//             test_and_print("offer Res: a", ull(_md->a), 1);
//             test_and_print("offer Res: b", ull(_md->b), 0);
//             test_and_print("offer Res: c", ull(_md->c), 0);
//             con_next_cycle(6);
//             test_and_print("offer Res: a", ull(_md->a), 1);
//             test_and_print("offer Res: b", ull(_md->b), 1);
//             test_and_print("offer Res: c", ull(_md->c), 0);
//             con_next_cycle(6);
//             std::cout << TC_BLUE << "case b is accept offer c must not get the offer" << TC_DEF << std::endl;
//             test_and_print("offer Res: st2", ull(_md->st2), 0); //// because it is wire
//             test_and_print("offer Res: a", ull(_md->a), 1);
//             test_and_print("offer Res: b", ull(_md->b), 2);
//             test_and_print("offer Res: c", ull(_md->c), 0);
//         }
//
//
//
//     };
//
//
//     class Sim49TestEle: public AutoTestEle{
//     public:
//         explicit Sim49TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod49, 1);
//             start_model_kathryn();
//             sim49 simulator((test_sim_mod49*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim49TestEle ele49(49);
// }