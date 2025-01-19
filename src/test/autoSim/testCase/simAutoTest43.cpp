//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "lib/hw/slot/table.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod43: public Module{
    public:
        Table testTable;
        mWire(validInput, 1);
        mWire(xInput    , 8);
        mReg(setIdx     , 8);
        ///// min gather
        mWire(searchValue  , 8);
        mVal(searchIdx , testTable._identWidth, 2);
        Wire* setEnable = nullptr;

        explicit testSimMod43(int x):
        testTable(
        {"valid", "x"},
         {1     , 8},
        "myTestTable",
        2){}

        void flow() override{

            /**build set logic first*/
            SlotOpr inputOpr(testTable._meta, {&validInput, &xInput});
            setEnable = &testTable.buildSetLogic(inputOpr, setIdx(0, testTable._identWidth));

            /**build min logic */
            auto outputOpr   = testTable.buildGetLogic(searchIdx); ////// find max
                 searchValue =  outputOpr.getOpr("x");


            /**set value*/
            seq{
                setIdx = 255;
                cdowhile(setIdx < 3){par{ //// we last idx is 3
                    setIdx     = setIdx + 1;
                    xInput     = setIdx + 1;
                    validInput = 1;
                    *setEnable = 1;
                }}
            }
        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest43.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest43.vcd";


    class sim43 :public SimAutoInterface{
    public:

        testSimMod43* _md;

        sim43(testSimMod43* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{
            conNextCycle(4);  //// 3+1 we check wire we must advance to end of the cycle
            testAndPrint("check searchIdx Id 2 x: search must be 0", (ull)(_md->searchValue)  , 0);
            conNextCycle(1);
            testAndPrint("check searchIdx Id 2 x: search must be 3", (ull)(_md->searchValue)  , 3);
        }

    };


    class Sim43TestEle: public AutoTestEle{
    public:
        explicit Sim43TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod43, 1);
            startModelKathryn();
            sim43 simulator((testSimMod43*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim43TestEle ele43(43);
}