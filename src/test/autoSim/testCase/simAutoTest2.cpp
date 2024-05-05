//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod2: public Module{
    public:
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mVal(iv, 8, 0b10101010);

        explicit testSimMod2(int x): Module(){}

        void flow() override{

            mVal(bnk, 8, 48);
            mVal(one, 8, 1);
            mVal(two, 8, 2);
            mVal(maxer, 8, 255);
            mReg(ota, 8);
            mReg(maxReg, 8);
            mReg(bwOrReg, 8);


            par{
                a <<= iv;
                b <<= iv;
                c <<= bnk;
                ota <<= bnk - one;
                maxReg <<= maxer + two;
                bwOrReg <<= bnk | (one & one);
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest2.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest2.vcd";


    class sim2 :public SimAutoInterface{
    public:

        testSimMod2* _md;

        sim2(testSimMod2* md):SimAutoInterface(2,
                                100,
                                vcdPath,
                                profilePath),
                                _md(md)
        {}

        void simAssert() override{


            incCycle(3);

            sim {
                ValRep testVal = NumConverter::createValRep(8, 7);
                testAndPrint("check End Val", _md->a.sv(), testVal);
            };
        }

        void simDriven() override{
            incCycle(2);

            sim {
                _md->iv.sv() = NumConverter::createValRep(8, 7);
            };
            incCycle(1);
        }

    };


    class Sim2TestEle: public AutoTestEle{
    public:
        explicit Sim2TestEle(int id): AutoTestEle(id){}
        void start() override{
            mMod(d, testSimMod2, 1);
            startModelKathryn();
            sim2 simulator((testSimMod2*) &d);
            simulator.simStart();
        }

    };

    Sim2TestEle ele2(2);

    ///sim2 testCase2;




}