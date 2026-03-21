// //
// // Created by tanawin on 22/1/2567.
// //
//
// #include "kathryn.h"
// //#include "lib/hw/slot/table.h"
// #include "test/auto_sim/sim_auto_interface.h"
// #include "auto_test_interface.h"
// //#include "carolyne/arch/caro/caro_repo.h"
//
// namespace kathryn{
//
//     class test_sim_mod45: public Module{
//     public:
//         std::vector<Reg*> counters;
//         explicit test_sim_mod45(int counter_size){
//             assert(counter_size >= 1);
//             for (int idx = 0; idx < counter_size; idx++){
//                 counters.push_back(&m_opr_reg("r" + std::to_string(idx), 32));
//             }
//         }
//
//         void flow() override{
//
//             cwhile(true){
//                 for (auto* counter: counters){
//                     *counter = *counter + 1;
//                 }
//             }
//         }
//     };
//
//     ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test45.vcd";
//     ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test45.vcd";
//
//
//     class sim45 :public SimAutoInterface{
//     public:
//
//         std::chrono::time_point<std::chrono::steady_clock> start_time;
//         std::chrono::time_point<std::chrono::steady_clock> stop_time;
//
//         test_sim_mod45* _md;
//
//         sim45(test_sim_mod45* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
//                                               20000000,
//                                               prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                               prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//                              _md(md){
//             enable_lrc();
//             disable_con_sim();
//         }
//
//         void print_front_back_counter() const{
//
//             std::cout << "couter_number 0 : count to " << (ull)(*_md->counters[0]) << std::endl;
//
//             std::cout << "couter_number " << std::to_string(_md->counters.size()-1) << " : count to " << (ull)(*_md->counters[_md->counters.size()-1]) << std::endl;
//         }
//
//         void describe_model_trigger() override{
//             trig( (*_md->counters[0]) == 1000000, [&](){
//                 stop_time = std::chrono::steady_clock::now();
//                 std::chrono::duration<double> elapsed_seconds = stop_time - start_time;
//                 std::cout << "process time: " << elapsed_seconds.count() << "s\n";
//                 print_front_back_counter();
//                 get_sim_controller_ptr()->stop_sim();
//
//             });
//         }
//
//         void describe_def() override{
//             SimInterface::describe_def();
//             start_time = std::chrono::steady_clock::now();
//         }
//
//     };
//
//
//     class Sim45TestEle: public AutoTestEle{
//         std::chrono::time_point<std::chrono::steady_clock> start_time;
//         std::chrono::time_point<std::chrono::steady_clock> stop_time;
//
//     public:
//         explicit Sim45TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod45, 4000);
//             start_model_kathryn();
//             sim45 simulator((test_sim_mod45*) &d, _simId, prefix, sim_proxy_build_mode);
//             simulator.sim_start();
//
//
//         }
//
//     };
//
//     //Sim45TestEle ele45(45);
// }
