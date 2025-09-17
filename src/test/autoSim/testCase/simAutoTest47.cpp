// //
// // Created by tanawin on 23/3/2025.
// //
//
//
// #include "kathryn.h"
// #include "test/autoSim/simAutoInterface.h"
// #include "autoTestInterface.h"
//
//
//
// namespace kathryn{
//
//     class testSimMod47: public Module{
//     public:
//         mReg(a, 8);
//         mReg(b, 8);
//         mReg(c, 8);
//         mReg(d, 8);
//         mReg(e, 8);
//         mReg(f, 8);
//         mWire(intRst, 1);
//         mWire(intSta, 1);
//
//         explicit testSimMod47(int x): Module(){}
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
//                         pipTran("decode"){a <<= a + 1;}
//                     }
//                     ///// decode pipe
//                     pip("decode"){ intrReset(intRst); intrStart(intSta);
//                         pipTran("exec"){b <<= a;}
//                     }
//
//                     pip("exec"){
//                         c <<= b;
//                     }
//
//                     seq{
//                         syWait(4);
//                         intRst = 1;
//                         syWait(4);
//                         intSta = 1;
//
//                     }
//                 }
//
//             }
//         }
//
//     };
//
//     ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest47.vcd";
//     ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest47.vcd";
//
//
//     class sim47 :public SimAutoInterface{
//     public:
//
//         testSimMod47* _md;
//
//         sim47(testSimMod47* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
//                                                  300,
//                                                  prefix + "simAutoResult"+std::to_string(idx)+".vcd",
//                                                  prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
//                                 _md(md)
//         {}
//
//         void describeCon() override{
//             std::cout << TC_BLUE << "test pipe with interupt reset" << TC_DEF << std::endl;
//             conNextCycle(2);
//             testAndPrint("testPipVal: B", ull(_md->b), 0);
//             conNextCycle(1);
//             testAndPrint("testPipVal: B", ull(_md->b), 1);
//             conNextCycle(1);
//             testAndPrint("testPipVal: B", ull(_md->b), 2);
//             conNextCycle(1);
//             testAndPrint("testPipVal: B", ull(_md->b), 3);
//             conNextCycle(1);
//             testAndPrint("testPipVal: B hold", ull(_md->b), 3);
//             conNextCycle(6);
//             testAndPrint("testPipVal: B renew", ull(_md->b), 5);
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
//         void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
//             mMod(d, testSimMod47, 1);
//             startModelKathryn();
//             sim47 simulator((testSimMod47*) &d, _simId, prefix, simProxyBuildMode);
//             simulator.simStart();
//         }
//
//     };
//
//     Sim47TestEle ele47(47);
// }