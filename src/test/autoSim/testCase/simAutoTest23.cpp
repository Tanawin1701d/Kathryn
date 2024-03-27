//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod23: public Module{
    public:
        makeReg(a, 32);
        makeReg(b, 32);
        makeReg(c, 32);
        makeReg(d, 32);

        explicit testSimMod23(int x): Module(){}

        void flow() override{

            pipWrap{
                pipBlk{
                    a <<= a + 1;
                }
                pipBlk{
                    b <<= a;
                }
                pipBlk{
                    c <<= b;
                };
                pipBlk{
                    d <<= c;
                }
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest23.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest23.vcd";


    class sim23 :public SimAutoInterface{
    public:

        testSimMod23* _md;

        sim23(testSimMod23* md):SimAutoInterface(23,
                                                 300,
                                                 vcdPath,
                                                 profilePath),
                                _md(md)
        {}

        void describeCon() override{
//            testAndPrint("test dry nest", ull(_md->m), (1 << 9) + 8);
//            conEndCycle();
//            testAndPrint("test memAndWire dummy", ull(_md->myStOutOld), 0);
//            testAndPrint("test memAndWire fill" , ull(_md->myStOut), 8);
        }



    };


    class Sim23TestEle: public AutoTestEle{
    public:
        explicit Sim23TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod23, 1);
            startModelKathryn();
            sim23 simulator((testSimMod23*) &d);
            simulator.simStart();
        }

    };

    Sim23TestEle ele23(23);
}