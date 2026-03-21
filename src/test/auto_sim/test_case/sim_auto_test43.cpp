// //
// // Created by tanawin on 22/1/2567.
// //
//
// #include "kathryn.h"
// #include "lib/hw/slot/table.h"
// #include "test/auto_sim/sim_auto_interface.h"
// #include "auto_test_interface.h"
// #include "carolyne/arch/caro/caro_repo.h"
//
// namespace kathryn{
//
//     class test_sim_mod43: public Module{
//     public:
//         Table test_table;
//         m_wire(valid_input, 1);
//         m_wire(x_input    , 8);
//         m_reg(set_idx     , 8);
//         ///// min gather
//         m_wire(search_value  , 8);
//         m_val(search_idx , test_table._identWidth, 2);
//         Wire* set_enable = nullptr;
//
//         explicit test_sim_mod43(int x):
//         test_table(
//         {"valid", "x"},
//          {1     , 8},
//         "my_test_table",
//         2){}
//
//         void flow() override{
//
//             /**build set logic first*/
//             Slot input(test_table._meta, -1,{{&valid_input}, {&x_input}});
//             set_enable = &test_table.build_set_logic(input, set_idx(0, test_table._identWidth));
//
//             /**build min logic */
//             auto output_opr   = test_table.build_get_logic(search_idx); ////// find max
//                  search_value =  output_opr.get_field("x");
//
//
//             /**set value*/
//             seq{
//                 set_idx = 255;
//                 cdowhile(set_idx < 3){par{ //// we last idx is 3
//                     set_idx     = set_idx + 1;
//                     x_input     = set_idx + 1;
//                     valid_input = 1;
//                     *set_enable = 1;
//                 }}
//             }
//         }
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test43.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test43.vcd";
//
//
//     class sim43 :public SimAutoInterface{
//     public:
//
//         test_sim_mod43* _md;
//
//         sim43(test_sim_mod43* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                               200,
//                                               prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                               prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                              _md(md)
//         {}
//
//         void describe_con() override{
//             con_next_cycle(4);  //// 3+1 we check wire we must advance to end of the cycle
//             test_and_print("check search_idx Id 2 x: search must be 0", (ull)(_md->search_value)  , 0);
//             con_next_cycle(1);
//             test_and_print("check search_idx Id 2 x: search must be 3", (ull)(_md->search_value)  , 3);
//         }
//
//     };
//
//
//     class Sim43TestEle: public AutoTestEle{
//     public:
//         explicit Sim43TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod43, 1);
//             start_model_kathryn();
//             sim43 simulator((test_sim_mod43*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim43TestEle ele43(43);
// }