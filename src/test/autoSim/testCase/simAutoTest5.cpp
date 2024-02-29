//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod5: public Module{
    public:
        bool testAutoSkip = false;
        makeVal(bnk, 32, 48);
        makeVal(zero, 32, 0);
        makeReg(a , 32);
        makeReg(b , 32);

        explicit testSimMod5(bool testAutoSkip): Module(){}

        void flow() override{
            Val* valueToused = testAutoSkip ? &zero : &bnk;

            seq{
                a <<= zero;
                cwhile(a < (*valueToused)){
                    makeVal(one, 32, 1);
                    a <<= a + one;
                }
                b <<= bnk;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest5.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest5.vcd";


    class sim5 :public SimAutoInterface{
    public:

        testSimMod5* _md;

        sim5(testSimMod5* md):SimAutoInterface(2,
                                              100,
                                              vcdPath,
                                               profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(53);

            sim {
                ValRep testVal = NumConverter::createValRep(32, 48);
                testAndPrint("check End loop", _md->b.sv(), testVal);
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


    class Sim5TestEle: public AutoTestEle{
    public:
        explicit Sim5TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod5, 1);
            sim5 simulator((testSimMod5*) &d);
            simulator.simStart();
        }

    };

    Sim5TestEle ele5(5);

    ///sim2 testCase2;




}