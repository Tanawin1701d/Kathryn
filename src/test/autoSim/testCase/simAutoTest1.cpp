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

            seq{
                a <<= iv;
                b <<= a;
                c <<= b;
                makeVal(zero, 8, 0);
                a <<= zero;
                b <<= zero;
                c <<= zero;
                for (int i = 0; i < 4; i++){
                    makeVal(x, 8, i+1);
                    a <<= x;
                }
                makeVal(y, 8, 3);
                a <<= a << y;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest1.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest1.vcd";

    class sim1 :public SimAutoInterface{
    public:

        testSimMod* _md = nullptr;

        sim1(testSimMod* md):SimAutoInterface(0,100,vcdPath, profilePath),
        _md(md)
        {}

        void simAssert() override{
            incCycle(13);
            sim {
                ValRep testVal = NumConverter::cvtStrToValRep(8, 32);
                testAndPrint("check End Val", _md->a.sv(), testVal);
            };
        }

        void simDriven() override{
            incCycle(2);
            for (int i = 0; i < 100; i++) {
                sim {
                    _md->iv.sv() = NumConverter::cvtStrToValRep(8, 7);
                };
                incCycle(1);
            }
        }
    };

    class Sim1TestEle: public AutoTestEle{
    public:
        explicit Sim1TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod, 1);
            sim1 simulator((testSimMod*) &d);
            simulator.simStart();
        }

    };

    Sim1TestEle ele1(1);

}