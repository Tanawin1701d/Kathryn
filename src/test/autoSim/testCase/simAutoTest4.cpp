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

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest4.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest4.vcd";


    class sim4 :public SimAutoInterface{
    public:

        testSimMod4* _md;

        sim4(testSimMod4* md):SimAutoInterface(2,
                                              100,
                                              vcdPath,
                                               profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(7);

            sim {
                ValRep testVal = NumConverter::createValRep(8, 48);
                testAndPrint("check End Val", _md->d1.sv(), testVal);
            };

            setCycle(6);

            sim{
                ValRep testVal = NumConverter::createValRep(8, 64);
                testAndPrint("check End Val", _md->c2.sv(), testVal);
            };
        }

        void simDriven() override{
            incCycle(2);

            sim {
                _md->iv.sv() = NumConverter::createValRep(8, 48);
            };
            incCycle(1);
        }

    };


    class Sim4TestEle: public AutoTestEle{
    public:
        explicit Sim4TestEle(int id): AutoTestEle(id){}
        void start() override{
            mMod(d, testSimMod4, 1);
            startModelKathryn();
            sim4 simulator((testSimMod4*) &d);
            simulator.simStart();
        }

    };

    Sim4TestEle ele4(4);

    ///sim2 testCase2;




}