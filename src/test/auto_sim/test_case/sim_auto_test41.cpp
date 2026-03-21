 // //
 // // Created by tanawin on 22/1/2567.
 // //
 //
 // #include "kathryn.h"
 // #include "lib/hw/slot/table.h"
 // #include "test/auto_sim/sim_auto_interface.h"
 // #include "auto_test_interface.h"
 //
 // namespace kathryn{
 //
 //     class test_sim_mod41: public Module{
 //     public:
 //         Table test_table;
 //         m_wire(valid_input, 1);
 //         m_wire(x_input    , 20);
 //         m_wire(y_input    , 12);
 //         m_reg(set_idx     , test_table._identWidth);
 //         m_val(z1         , 20, 0);
 //
 //         explicit test_sim_mod41(int x):
 //         test_table(
 //         "my_test_table",
 //         {"valid", "x", "y"},
 //         {1      ,  20, 12 },
 //         1){}
 //
 //         void flow() override{
 //
 //             /**build set logic first*/
 //             Slot input_opr({"valid"    , "x"    , "y"    },
 //                           {&valid_input, &x_input, &y_input});
 //
 //             /**set value*/
 //             seq{
 //                 set_idx <<= 0;
 //                 cdowhile(set_idx < 1){
 //                     par{
 //                         valid_input    = 1;
 //                         x_input        = g(z1, set_idx) + 5;
 //                         y_input        = g(z1, set_idx) + 5;
 //                         set_idx      <<= set_idx + 1;
 //                         test_table.assign(input_opr, set_idx, true);
 //                     }
 //                 }
 //             }
 //         }
 //     };
 //
 //     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test41.vcd";
 //     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test41.vcd";
 //
 //
 //     class sim41 :public SimAutoInterface{
 //     public:
 //
 //         test_sim_mod41* _md;
 //
 //         sim41(test_sim_mod41* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
 //                                               200,
 //                                               prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
 //                                               prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
 //                              _md(md)
 //         {}
 //
 //         void describe_con() override{
 //             con_next_cycle(2);
 //             test_and_print("check_table[0] init valid: ", (ull)(_md->test_table._hwSlots[0]->get("valid")), 1);
 //             test_and_print("check_table[0] init x: "    , (ull)(_md->test_table._hwSlots[0]->get("x"))    , 5);
 //             test_and_print("check_table[0] init y: "    , (ull)(_md->test_table._hwSlots[0]->get("y"))    , 5);
 //             test_and_print("check_table[1] init valid: ", (ull)(_md->test_table._hwSlots[1]->get("valid")), 0);
 //             test_and_print("check_table[1] init x: "    , (ull)(_md->test_table._hwSlots[1]->get("x"))    , 0);
 //             test_and_print("check_table[1] init y: "    , (ull)(_md->test_table._hwSlots[1]->get("y"))    , 0);
 //             con_next_cycle(1);
 //             test_and_print("check_table[0] init valid: ", (ull)(_md->test_table._hwSlots[0]->get("valid")), 1);
 //             test_and_print("check_table[0] init x: "    , (ull)(_md->test_table._hwSlots[0]->get("x"))    , 5);
 //             test_and_print("check_table[0] init y: "    , (ull)(_md->test_table._hwSlots[0]->get("y"))    , 5);
 //             test_and_print("check_table[1] init valid: ", (ull)(_md->test_table._hwSlots[1]->get("valid")), 1);
 //             test_and_print("check_table[1] init x: "    , (ull)(_md->test_table._hwSlots[1]->get("x"))    , 6);
 //             test_and_print("check_table[1] init y: "    , (ull)(_md->test_table._hwSlots[1]->get("y"))    , 6);
 //         }
 //
 //     };
 //
 //
 //     class Sim41TestEle: public AutoTestEle{
 //     public:
 //         explicit Sim41TestEle(int id): AutoTestEle(id){}
 //         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
 //             m_mod(d, test_sim_mod41, 1);
 //             start_model_kathryn();
 //             sim41 simulator((test_sim_mod41*) &d, _simId, prefix, sim_proxy_build_mode);
 //             simulator.sim_start();
 //         }
 //
 //     };
 //
 //     Sim41TestEle ele41(41);
 // }