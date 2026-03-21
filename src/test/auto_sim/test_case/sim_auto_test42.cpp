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
 //     class test_sim_mod42: public Module{
 //     public:
 //         Table test_table;
 //         m_wire(valid_input, 1);
 //         m_wire(x_input    , 8);
 //         m_reg(set_idx     , 8);
 //         ///// min gather
 //         m_wire(max_idx    , test_table._identWidth);
 //         m_wire(max_value  , 8);
 //
 //         explicit test_sim_mod42(int x):
 //         test_table(
 //         "my_test_table42",
 //         {"valid", "x"},
 //         {1      ,   8},
 //         2){}
 //
 //         void flow() override{
 //
 //             /**build set logic first*/
 //             Slot input_slot({"valid", "x"},
 //                            { &valid_input, &x_input});
 //
 //             /**build min logic */
 //             auto[max_slot, max_idx_opr] = test_table.build_min_max_logic("x", true, false); ////// find max
 //             max_idx   = *max_idx_opr;
 //             max_value = max_slot.get_field("x");
 //
 //
 //             /**set value*/
 //             seq{
 //                 set_idx = 0;
 //                 cdowhile(set_idx < 3){par{ //// we last idx is 3
 //                     valid_input =   1;
 //                     x_input     =   set_idx + 5;
 //                     set_idx     <<= set_idx + 1;
 //                     test_table.assign(input_slot, set_idx(0,2), true);
 //                 }}
 //             }
 //         }
 //     };
 //
 //     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test42.vcd";
 //     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test42.vcd";
 //
 //
 //     class sim42 :public SimAutoInterface{
 //     public:
 //
 //         test_sim_mod42* _md;
 //
 //         sim42(test_sim_mod42* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
 //                                               200,
 //                                               prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
 //                                               prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
 //                              _md(md)
 //         {}
 //
 //         void describe_con() override{
 //             con_next_cycle(2+1);  //// 2+1 we check wire we must advance to end of the cycle
 //
 //             test_and_print("max_idx "  , (ull)(_md->max_idx  ), 0);
 //             test_and_print("maxvalue ", (ull)(_md->max_value), 5);
 //             con_next_cycle(1);
 //             test_and_print("max_idx "  , (ull)(_md->max_idx  ), 1);
 //             test_and_print("maxvalue ", (ull)(_md->max_value), 6);
 //             con_next_cycle(1);
 //             test_and_print("max_idx "  , (ull)(_md->max_idx  ), 2);
 //             test_and_print("maxvalue ", (ull)(_md->max_value), 7);
 //             con_next_cycle(1);
 //             test_and_print("max_idx "  , (ull)(_md->max_idx  ), 3);
 //             test_and_print("maxvalue ", (ull)(_md->max_value), 8);
 //
 //         }
 //
 //     };
 //
 //
 //     class Sim42TestEle: public AutoTestEle{
 //     public:
 //         explicit Sim42TestEle(int id): AutoTestEle(id){}
 //         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
 //             m_mod(d, test_sim_mod42, 1);
 //             start_model_kathryn();
 //             sim42 simulator((test_sim_mod42*) &d, _simId, prefix, sim_proxy_build_mode);
 //             simulator.sim_start();
 //         }
 //
 //     };
 //
 //     Sim42TestEle ele42(42);
 // }