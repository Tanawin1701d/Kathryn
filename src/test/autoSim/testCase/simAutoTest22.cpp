//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod22: public Module{
    public:
        makeReg(a, 32);
        makeReg(m, 32);
        makeReg(b, 32);

        makeVal(c, 8, 8);
        makeVal(d, 32, 10);

        makeWire(i, 6);
        makeMem(mySt, 48, 8);
        makeWire(myStOut, 8);
        makeWire(myStOutOld, 8);


        explicit testSimMod22(int x): Module(){}

        void flow() override{

            g(a,m,b) <<= g(c,d);
            m(9) <<= 1;
            /**value*/
            i          = 8;
            mySt[i]    <<= c;
            myStOut    = mySt[i];
            myStOutOld = mySt[0];

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest22.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest22.vcd";


    class sim22 :public SimAutoInterface{
    public:

        testSimMod22* _md;

        sim22(testSimMod22* md):SimAutoInterface(22,
                                              300,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

    };


    class Sim22TestEle: public AutoTestEle{
    public:
        explicit Sim22TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod22, 1);
            sim22 simulator((testSimMod22*) &d);
            simulator.simStart();
        }

    };

    Sim22TestEle ele22(-1);
}