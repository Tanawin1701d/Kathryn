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
        mVal(iv, 8, 7);///0b10101010);

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

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest2.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest2.vcd";


    class sim2 :public SimAutoInterface{
    public:

        testSimMod2* _md;

        sim2(testSimMod2* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                100,
                                prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                prefix + "simAutoResult"+std::to_string(idx)+".prof",
                                simProxyBuildMode),
                                _md(md)
        {}

        void simAssert() override{


            incCycle(3);

            sim {
                ull testVal = 7;
                testAndPrint("check End Val", (ull)_md->a, 7);
            };
        }

        void simDriven() override{
            // incCycle(2);
            //
            // sim {
            //     _md->iv = 7;
            // };
            // incCycle(1);
        }

    };


    class Sim2TestEle: public AutoTestEle{
    public:
        explicit Sim2TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod2, 1);
            startModelKathryn();
            sim2 simulator((testSimMod2*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim2TestEle ele2(2);

    ///sim2 testCase2;




}