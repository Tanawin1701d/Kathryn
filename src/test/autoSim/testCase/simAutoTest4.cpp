//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod4: public Module{
    public:
        mReg(a0, 8);
        mReg(b0, 8);
        /** lane1*/
        mReg(a1, 8);
        mReg(b1, 8);
        mReg(c1, 8);
        mReg(d1, 8);
        /** lane2*/
        mReg(a2, 8);
        mReg(b2, 8);
        mReg(c2, 8);

        mVal(iv,   8,48);
        mVal(iv2,  8,64);
        mVal(zero, 8, 0);


        explicit testSimMod4(int x): Module(){}

        void flow() override{
            seq {
                a0 <<= iv;
                par {
                    seq {
                        a1 <<= iv;
                        b1 <<= a1;
                        c1 <<= b1;
                        d1 <<= c1;
                    }
                    seq {
                        a2 <<= iv2;
                        b2 <<= a2;
                        c2 <<= b2;
                    }
                }
                b0 <<= iv;
            }

        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest4.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest4.vcd";


    class sim4 :public SimAutoInterface{
    public:

        testSimMod4* _md;

        sim4(testSimMod4* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              100,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                               prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(7);

            sim {
                ull testVal = 48;
                testAndPrint("check End Val", (ull)_md->d1, testVal);
            };

            setCycle(6);

            sim{
                ull testVal = 64;
                testAndPrint("check End Val", (ull)_md->c2, testVal);
            };
        }

        void simDriven() override{
            incCycle(2);

            sim {
                _md->iv = 48;
            };
            incCycle(1);
        }

    };


    class Sim4TestEle: public AutoTestEle{
    public:
        explicit Sim4TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod4, 1);
            startModelKathryn();
            sim4 simulator((testSimMod4*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim4TestEle ele4(4);

    ///sim2 testCase2;




}