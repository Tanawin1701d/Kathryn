//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod21: public Module{
    public:
        makeReg(a, 32);
        makeWire(result, 1);

        explicit testSimMod21(int x): Module(){}

        void flow() override{

            seq {
                a <<= 16;
                syWait(a);
                result = 1;
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest21.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest21.vcd";


    class sim21 :public SimAutoInterface{
    public:

        testSimMod21* _md;

        sim21(testSimMod21* md):SimAutoInterface(21,
                                              300,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void describeCon() override{
            for(int i = 0; i < 17; i++){
                if ((i % 4) == 0) {
                    conEndCycle();
                    testAndPrint("testResult is waiting", ull(_md->result), 0);
                }
                conNextCycle(1);
            }
            conEndCycle();
            testAndPrint("testResult is waiting set", ull(_md->result), 1);

        }

    };


    class Sim21TestEle: public AutoTestEle{
    public:
        explicit Sim21TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod21, 1);
            startModelKathryn();
            sim21 simulator((testSimMod21*) &d);
            simulator.simStart();
        }

    };

    Sim21TestEle ele21(21);
}