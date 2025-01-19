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
        mVal(z1         , 20);
        Wire* setEnable = nullptr;

        explicit testSimMod41(int x):
        testTable(
        {"valid", "x", "y"},
         {1      ,  20, 12 },
        "myTestTable",
        1){}

        void flow() override{

            /**build set logic first*/
            SlotOpr inputOpr(testTable._meta,
                {&validInput, &xInput, &yInput});
            setEnable = &testTable.buildSetLogic(inputOpr, setIdx);

            /**set value*/
            seq{
                setIdx = 0;
                cdowhile(setIdx < 1){
                    par{
                        validInput    = 1;
                        xInput        = g(z1, setIdx) + 1;
                        yInput        = g(z1, setIdx) + 1;
                        *setEnable    = 1;
                        setIdx      <<= setIdx + 1;

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
            testAndPrint("checkTable[0] init valid: ", (ull)(*_md->testTable._hwSlots[0]->hwFields[0]), 0);
            testAndPrint("checkTable[0] init x: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[1]), 0);
            testAndPrint("checkTable[0] init y: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[2]), 0);
            testAndPrint("checkTable[1] init valid: ", (ull)(*_md->testTable._hwSlots[1]->hwFields[0]), 0);
            testAndPrint("checkTable[1] init x: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[1]), 0);
            testAndPrint("checkTable[1] init y: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[2]), 0);
            conNextCycle(2);
            testAndPrint("checkTable[0] proc valid: ", (ull)(*_md->testTable._hwSlots[0]->hwFields[0]), 1);
            testAndPrint("checkTable[0] proc x: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[1]), 1);
            testAndPrint("checkTable[0] proc y: "    , (ull)(*_md->testTable._hwSlots[0]->hwFields[2]), 1);
            conNextCycle(1);
            testAndPrint("checkTable[1] proc valid: ", (ull)(*_md->testTable._hwSlots[1]->hwFields[0]), 1);
            testAndPrint("checkTable[1] proc x: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[1]), 2);
            testAndPrint("checkTable[1] proc y: "    , (ull)(*_md->testTable._hwSlots[1]->hwFields[2]), 2);
//            for(int i = 0; i < 48; i++){
//                ValRep testVal = NumConverter::createValRep(6, i);
//
//                testAndPrint("checkMem : " + std::to_string(i), _md->b.sv(), testVal);
//                conNextCycle(1);
//            }
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