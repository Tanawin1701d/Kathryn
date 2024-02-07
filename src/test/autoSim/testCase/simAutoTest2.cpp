//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod: public Module{
    public:
        makeReg(a, 8);
        makeReg(b, 8);
        makeReg(c, 8);
        makeVal(iv, 8, 0b10101010);

        explicit testSimMod(int x): Module(){}

        void flow() override{

            makeVal(bnk, 8, 48);
            makeVal(one, 8, 1);
            makeVal(two, 8, 2);
            makeVal(maxer, 8, 255);
            makeReg(ota, 8);
            makeReg(maxReg, 8);
            makeReg(bwOrReg, 8);


            par{
                a <<= iv;
                b <<= iv;
                c <<= bnk;
                ota <<= bnk - one;
                maxReg <<= maxer + two;
                bwOrReg <<= bnk | one;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest2.vcd";

    class sim2 :public SimAutoInterface{
    public:

        testSimMod* _md;

        sim2(testSimMod* md):SimAutoInterface(2,
                                100,
                                vcdPath),
                                _md(md)
        {}

        void simAssert() override{


            incCycle(3);

            sim {
                ValRep testVal = NumConverter::cvtStrToValRep(8, 7);
                testAndPrint("check End Val", _md->a.sv(), testVal);
            };
        }

        void simDriven() override{
            incCycle(2);

            sim {
                _md->iv.sv() = NumConverter::cvtStrToValRep(8, 7);
            };
            incCycle(1);
        }

    };


    class Sim2TestEle: public AutoTestEle{
    public:
        explicit Sim2TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod, 1);
            sim2 simulator((testSimMod*) &d);
            simulator.simStart();
        }

    };

    Sim2TestEle ele2(2);

    ///sim2 testCase2;




}