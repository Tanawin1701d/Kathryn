 //
 // Created by tanawin on 22/1/2567.
 //

 #include "kathryn.h"
 #include "lib/hw/slot/table.h"
 #include "test/autoSim/simAutoInterface.h"
 #include "autoTestInterface.h"

 namespace kathryn{

     class testSimMod41: public Module{
     public:
         Table testTable;
         mWire(validInput, 1);
         mWire(xInput    , 20);
         mWire(yInput    , 12);
         mReg(setIdx     , testTable._identWidth);
         mVal(z1         , 20, 0);

         explicit testSimMod41(int x):
         testTable(
         "myTestTable",
         {"valid", "x", "y"},
         {1      ,  20, 12 },
         1){}

         void flow() override{

             /**build set logic first*/
             Slot inputOpr({"valid"    , "x"    , "y"    },
                           {&validInput, &xInput, &yInput});

             /**set value*/
             seq{
                 setIdx <<= 0;
                 cdowhile(setIdx < 1){
                     par{
                         validInput    = 1;
                         xInput        = g(z1, setIdx) + 5;
                         yInput        = g(z1, setIdx) + 5;
                         setIdx      <<= setIdx + 1;
                         testTable.assign(inputOpr, setIdx, true);
                     }
                 }
             }
         }
     };

     ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest41.vcd";
     ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest41.vcd";


     class sim41 :public SimAutoInterface{
     public:

         testSimMod41* _md;

         sim41(testSimMod41* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                               200,
                                               prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                               prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                              _md(md)
         {}

         void describeCon() override{
             conNextCycle(2);
             testAndPrint("checkTable[0] init valid: ", (ull)(_md->testTable._hwSlots[0]->get("valid")), 1);
             testAndPrint("checkTable[0] init x: "    , (ull)(_md->testTable._hwSlots[0]->get("x"))    , 5);
             testAndPrint("checkTable[0] init y: "    , (ull)(_md->testTable._hwSlots[0]->get("y"))    , 5);
             testAndPrint("checkTable[1] init valid: ", (ull)(_md->testTable._hwSlots[1]->get("valid")), 0);
             testAndPrint("checkTable[1] init x: "    , (ull)(_md->testTable._hwSlots[1]->get("x"))    , 0);
             testAndPrint("checkTable[1] init y: "    , (ull)(_md->testTable._hwSlots[1]->get("y"))    , 0);
             conNextCycle(1);
             testAndPrint("checkTable[0] init valid: ", (ull)(_md->testTable._hwSlots[0]->get("valid")), 1);
             testAndPrint("checkTable[0] init x: "    , (ull)(_md->testTable._hwSlots[0]->get("x"))    , 5);
             testAndPrint("checkTable[0] init y: "    , (ull)(_md->testTable._hwSlots[0]->get("y"))    , 5);
             testAndPrint("checkTable[1] init valid: ", (ull)(_md->testTable._hwSlots[1]->get("valid")), 1);
             testAndPrint("checkTable[1] init x: "    , (ull)(_md->testTable._hwSlots[1]->get("x"))    , 6);
             testAndPrint("checkTable[1] init y: "    , (ull)(_md->testTable._hwSlots[1]->get("y"))    , 6);
         }

     };


     class Sim41TestEle: public AutoTestEle{
     public:
         explicit Sim41TestEle(int id): AutoTestEle(id){}
         void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
             mMod(d, testSimMod41, 1);
             startModelKathryn();
             sim41 simulator((testSimMod41*) &d, _simId, prefix, simProxyBuildMode);
             simulator.simStart();
         }

     };

     Sim41TestEle ele41(41);
 }