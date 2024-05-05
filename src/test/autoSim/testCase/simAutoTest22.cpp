//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod22: public Module{
    public:
        mReg(a, 32);
        mReg(m, 32);
        mReg(b, 32);

        mVal(c, 8, 8);
        mVal(d, 32, 10);

        mWire(i, 6);
        mMem(mySt, 48, 8);
        mWire(myStOut, 8);
        mWire(myStOutOld, 8);


        explicit testSimMod22(int x): Module(){}

        void flow() override{

            g(a,m,b) <<= g(c,d);
            m(9) <<= 1;
            /**value*/
            i          = 4;
            mySt[i]    <<= c;
            myStOut    = mySt[i];
            myStOutOld = mySt[0];

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest22.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest22.vcd";


    class sim22 :public SimAutoInterface{
    public:

        testSimMod22* _md;

        sim22(testSimMod22* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              300,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void describeCon() override{
            testAndPrint("test dry nest", ull(_md->m), (1 << 9) + 8);
            conEndCycle();
            testAndPrint("test memAndWire dummy", ull(_md->myStOutOld), 0);
            testAndPrint("test memAndWire fill" , ull(_md->myStOut), 8);
        }



    };


    class Sim22TestEle: public AutoTestEle{
    public:
        explicit Sim22TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod22, 1);
            startModelKathryn();
            sim22 simulator((testSimMod22*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim22TestEle ele22(22);
}