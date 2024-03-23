//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod19: public Module{
    public:
        makeReg(a, 32);
        makeReg(b, 32);
        makeReg(c, 32);
        makeWire(d, 32);
        makeReg(end, 1);


        explicit testSimMod19(int x): Module(){}

        void flow() override{

            seq {
                cwhile(a <= 48) {
                    seq {
                        b <<= 5;
                        d = 6;
                        a <<= a + 1;
                        cif(a == 16) {
                            par {
                                sbreak;
                                b <<= 15;
                            }
                        }
                        d = 7;
                    }
                }
                end <<= 1;
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest19.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest19.vcd";


    class sim19 :public SimAutoInterface{
    public:

        testSimMod19* _md;

        sim19(testSimMod19* md):SimAutoInterface(2,
                                              300,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void describeCon() override{
            for(int i = 0; i < (17*4); i++){

                if ((i % 10) == 0) {
                    testAndPrint("testEnd is not set", ull(_md->end), 0);
                }
                conNextCycle(1);
            }
            conNextCycle(1);
            testAndPrint("test End is set", ull(_md->end), 1);
            testAndPrint("test B if break is used", ull(_md->b), 15);
            conNextCycle(1);
        }

    };


    class Sim19TestEle: public AutoTestEle{
    public:
        explicit Sim19TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod19, 1);
            startModelKathryn();
            sim19 simulator((testSimMod19*) &d);
            simulator.simStart();
        }

    };

    Sim19TestEle ele19(19);
}