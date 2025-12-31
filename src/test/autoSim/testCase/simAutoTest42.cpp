 // //
 // // Created by tanawin on 22/1/2567.
 // //
 //
 // #include "kathryn.h"
 // #include "lib/hw/slot/table.h"
 // #include "test/autoSim/simAutoInterface.h"
 // #include "autoTestInterface.h"
 //
 // namespace kathryn{
 //
 //     class testSimMod42: public Module{
 //     public:
 //         Table testTable;
 //         mWire(validInput, 1);
 //         mWire(xInput    , 8);
 //         mReg(setIdx     , 8);
 //         ///// min gather
 //         mWire(maxIdx    , testTable._identWidth);
 //         mWire(maxValue  , 8);
 //
 //         explicit testSimMod42(int x):
 //         testTable(
 //         "myTestTable42",
 //         {"valid", "x"},
 //         {1      ,   8},
 //         2){}
 //
 //         void flow() override{
 //
 //             /**build set logic first*/
 //             Slot inputSlot({"valid", "x"},
 //                            { &validInput, &xInput});
 //
 //             /**build min logic */
 //             auto[maxSlot, maxIdxOpr] = testTable.buildMinMaxLogic("x", true, false); ////// find max
 //             maxIdx   = *maxIdxOpr;
 //             maxValue = maxSlot.getField("x");
 //
 //
 //             /**set value*/
 //             seq{
 //                 setIdx = 0;
 //                 cdowhile(setIdx < 3){par{ //// we last idx is 3
 //                     validInput =   1;
 //                     xInput     =   setIdx + 5;
 //                     setIdx     <<= setIdx + 1;
 //                     testTable.assign(inputSlot, setIdx(0,2), true);
 //                 }}
 //             }
 //         }
 //     };
 //
 //     ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest42.vcd";
 //     ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest42.vcd";
 //
 //
 //     class sim42 :public SimAutoInterface{
 //     public:
 //
 //         testSimMod42* _md;
 //
 //         sim42(testSimMod42* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
 //                                               200,
 //                                               prefix + "simAutoResult"+std::to_string(idx)+".vcd",
 //                                               prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
 //                              _md(md)
 //         {}
 //
 //         void describeCon() override{
 //             conNextCycle(2+1);  //// 2+1 we check wire we must advance to end of the cycle
 //
 //             testAndPrint("maxIdx "  , (ull)(_md->maxIdx  ), 0);
 //             testAndPrint("maxvalue ", (ull)(_md->maxValue), 5);
 //             conNextCycle(1);
 //             testAndPrint("maxIdx "  , (ull)(_md->maxIdx  ), 1);
 //             testAndPrint("maxvalue ", (ull)(_md->maxValue), 6);
 //             conNextCycle(1);
 //             testAndPrint("maxIdx "  , (ull)(_md->maxIdx  ), 2);
 //             testAndPrint("maxvalue ", (ull)(_md->maxValue), 7);
 //             conNextCycle(1);
 //             testAndPrint("maxIdx "  , (ull)(_md->maxIdx  ), 3);
 //             testAndPrint("maxvalue ", (ull)(_md->maxValue), 8);
 //
 //         }
 //
 //     };
 //
 //
 //     class Sim42TestEle: public AutoTestEle{
 //     public:
 //         explicit Sim42TestEle(int id): AutoTestEle(id){}
 //         void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
 //             mMod(d, testSimMod42, 1);
 //             startModelKathryn();
 //             sim42 simulator((testSimMod42*) &d, _simId, prefix, simProxyBuildMode);
 //             simulator.simStart();
 //         }
 //
 //     };
 //
 //     Sim42TestEle ele42(42);
 // }