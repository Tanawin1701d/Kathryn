//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "lib/hw/slot/table.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod42: public Module{
    public:
        Table testTable;
        mWire(validInput, 1);
        mWire(xInput    , 8);
        mReg(setIdx     , 8);
        ///// min gather
        mWire(maxIdx    , testTable._identWidth);
        mWire(maxValue  , 8);
        Wire* setEnable = nullptr;

        explicit testSimMod42(int x):
        testTable(
        {"valid", "x"},
         {1     , 8},
        "myTestTable",
        2){}

        void flow() override{

            /**build set logic first*/
            Slot inputSlot(testTable._meta, -1, { {&validInput}, {&xInput}});
            setEnable = &testTable.buildSetLogic(inputSlot, setIdx(0, testTable._identWidth));

            /**build min logic */
            auto[maxSlot, maxIdxOpr] = testTable.buildMinMaxLogic("x", true, false); ////// find max
            maxIdx   = *maxIdxOpr;
            maxValue = maxSlot.getField("x");


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

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest42.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest42.vcd";


    class sim42 :public SimAutoInterface{
    public:

        testSimMod42* _md;

        sim42(testSimMod42* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{
            conNextCycle(3);  //// 2+1 we check wire we must advance to end of the cycle
            for (int result = 0; result < 4; result++){
                testAndPrint("checkMaxId    x: add step: " + std::to_string(result), (ull)(_md->maxIdx)  , result  );
                testAndPrint("checkMaxValue x: add step: " + std::to_string(result), (ull)(_md->maxValue), result+1);
                conNextCycle(1);
            }
            // testAndPrint("checkTable[0] init valid: ", (ull)(*_md->testTable._hwSlots[0]->hwFields[0]), 0);
            // testAndPrint("checkTable[0] init x: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[1]), 0);
            // testAndPrint("checkTable[0] init y: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[2]), 0);
            // testAndPrint("checkTable[1] init valid: ", (ull)(*_md->testTable._hwSlots[1]->hwFields[0]), 0);
            // testAndPrint("checkTable[1] init x: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[1]), 0);
            // testAndPrint("checkTable[1] init y: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[2]), 0);
            // conNextCycle(2);
            // testAndPrint("checkTable[0] proc valid: ", (ull)(*_md->testTable._hwSlots[0]->hwFields[0]), 1);
            // testAndPrint("checkTable[0] proc x: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[1]), 1);
            // testAndPrint("checkTable[0] proc y: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[2]), 1);
            // conNextCycle(1);
            // testAndPrint("checkTable[1] proc valid: ", (ull)(*_md->testTable._hwSlots[1]->hwFields[0]), 1);
            // testAndPrint("checkTable[1] proc x: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[1]), 2);
            // testAndPrint("checkTable[1] proc y: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[2]), 2);
//            for(int i = 0; i < 48; i++){
//                ValRep testVal = NumConverter::createValRep(6, i);
//
//                testAndPrint("checkMem : " + std::to_string(i), _md->b.sv(), testVal);
//                conNextCycle(1);
//            }
        }

    };


    class Sim42TestEle: public AutoTestEle{
    public:
        explicit Sim42TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod42, 1);
            startModelKathryn();
            sim42 simulator((testSimMod42*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim42TestEle ele42(42);
}