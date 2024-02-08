//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod6: public Module{
    public:
        bool testAutoSkip = false;
        makeVal(bnk, 32, 48);
        makeVal(zero, 32, 0);
        makeReg(a , 32);
        makeReg(b , 32);

        explicit testSimMod6(bool testAutoSkip): Module(){}

        void flow() override{
            Val* valueToused = testAutoSkip ? &zero : &bnk;

            seq{
                a <<= bnk;
                b <<= zero;
                cwhile(a > zero){
                    makeVal(one, 32, 1);
                    a <<= a - one;
                }
                b <<= bnk;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest6.vcd";

    class sim6 :public SimAutoInterface{
    public:

        testSimMod6* _md;

        sim6(testSimMod6* md):SimAutoInterface(2,
                                              100,
                                              vcdPath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(54);

            sim {
                ValRep testVal = NumConverter::cvtStrToValRep(32, 48);
                testAndPrint("check end loop", _md->b.sv(), testVal);
            };

        }

        void simDriven() override{
            incCycle(2);

            sim {
                //_md->iv.sv() = NumConverter::cvtStrToValRep(8, 48);
            };
            incCycle(1);
        }

    };


    class Sim6TestEle: public AutoTestEle{
    public:
        explicit Sim6TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod6, 1);
            sim6 simulator((testSimMod6*) &d);
            simulator.simStart();
        }

    };

    Sim6TestEle ele6(6);

    ///sim2 testCase2;




}