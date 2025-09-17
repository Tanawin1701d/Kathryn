// //
// // Created by tanawin on 27/3/2567.
// //
//
// #include "kathryn.h"
// #include "test/autoSim/simAutoInterface.h"
// #include "autoTestInterface.h"
//
//
//
// namespace kathryn{
//
//     class testSimMod46: public Module{
//     public:
//         mReg(a, 8);
//         mReg(b, 8);
//         mReg(c, 8);
//         mReg(d, 8);
//         mReg(e, 8);
//         mReg(f, 8);
//         mWire(pipI, 1);
//
//         explicit testSimMod46(int x): Module(){}
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
//                     pip("fetch"){ autoStart
//                         pipTranWhen("decode", pipI){a <<= a + 1;}
//                     }
//                     ///// decode pipe
//                     pip("decode"){
//                         seq{
//                             b <<= a;
//                             cif((a + 1) == 3){
//                                 syWait(3);
//                             }
//                         }
//                     }
//
//                     seq{
//                         syWait(3);
//                         pipI = 1;
//
//                     }
//                 }
//
//             }
//         }
//
//     };
//
//     ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest46.vcd";
//     ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest46.vcd";
//
//
//     class sim46 :public SimAutoInterface{
//     public:
//
//         testSimMod46* _md;
//
//         sim46(testSimMod46* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "simAutoResult"+std::to_string(idx)+".vcd",
//                                                  prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
//                                 _md(md){
//         }
//
//         void describeCon() override{
//
//             conNextCycle(3);
//             testAndPrint("testPipVal: A", ull(_md->a), 0);
//             testAndPrint("testPipVal: B", ull(_md->b), 0);
//             conNextCycle(2);
//             testAndPrint("testPipVal: A", ull(_md->a), 1);
//             testAndPrint("testPipVal: B", ull(_md->b), 0);
//             conNextCycle(1);
//             testAndPrint("testPipVal: A", ull(_md->a), 1);
//             testAndPrint("testPipVal: B", ull(_md->b), 1);
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
//         void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
//             mMod(d, testSimMod46, 1);
//             startModelKathryn();
//             sim46 simulator((testSimMod46*) &d, _simId, prefix, simProxyBuildMode);
//             simulator.simStart();
//         }
//
//     };
//
//     Sim46TestEle ele46(46);
// }