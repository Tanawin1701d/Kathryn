// //
// // Created by tanawin on 22/1/2567.
// //
//
// #include "kathryn.h"
// //#include "lib/hw/slot/table.h"
// #include "test/autoSim/simAutoInterface.h"
// #include "autoTestInterface.h"
// //#include "carolyne/arch/caro/caro_repo.h"
//
// namespace kathryn{
//
//     class testSimMod45: public Module{
//     public:
//         std::vector<Reg*> counters;
//         explicit testSimMod45(int counterSize){
//             assert(counterSize >= 1);
//             for (int idx = 0; idx < counterSize; idx++){
//                 counters.push_back(&mOprReg("r" + std::to_string(idx), 32));
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
//     ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest45.vcd";
//     ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest45.vcd";
//
//
//     class sim45 :public SimAutoInterface{
//     public:
//
//         std::chrono::time_point<std::chrono::steady_clock> startTime;
//         std::chrono::time_point<std::chrono::steady_clock> stopTime;
//
//         testSimMod45* _md;
//
//         sim45(testSimMod45* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
//                                               20000000,
//                                               prefix + "simAutoResult"+std::to_string(idx)+".vcd",
//                                               prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
//                              _md(md){
//             enableLRC();
//             disableConSim();
//         }
//
//         void printFrontBackCounter() const{
//
//             std::cout << "couterNumber 0 : count to " << (ull)(*_md->counters[0]) << std::endl;
//
//             std::cout << "couterNumber " << std::to_string(_md->counters.size()-1) << " : count to " << (ull)(*_md->counters[_md->counters.size()-1]) << std::endl;
//         }
//
//         void describeModelTrigger() override{
//             trig( (*_md->counters[0]) == 1000000, [&](){
//                 stopTime = std::chrono::steady_clock::now();
//                 std::chrono::duration<double> elapsed_seconds = stopTime - startTime;
//                 std::cout << "process time: " << elapsed_seconds.count() << "s\n";
//                 printFrontBackCounter();
//                 getSimController()->stopSim();
//
//             });
//         }
//
//         void describeDef() override{
//             SimInterface::describeDef();
//             startTime = std::chrono::steady_clock::now();
//         }
//
//     };
//
//
//     class Sim45TestEle: public AutoTestEle{
//         std::chrono::time_point<std::chrono::steady_clock> startTime;
//         std::chrono::time_point<std::chrono::steady_clock> stopTime;
//
//     public:
//         explicit Sim45TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
//             mMod(d, testSimMod45, 4000);
//             startModelKathryn();
//             sim45 simulator((testSimMod45*) &d, _simId, prefix, simProxyBuildMode);
//             simulator.simStart();
//
//
//         }
//
//     };
//
//     //Sim45TestEle ele45(45);
// }
