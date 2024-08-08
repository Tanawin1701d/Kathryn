//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod38: public Module{
    public:
        mVal(bnk, 32, 48);
        mVal(zero, 32, 0);
        mReg(a , 32);
        mReg(b , 32);

        mReg(r1, 32);
        mReg(r2, 32);
        mReg(rend, 32);

        explicit testSimMod38(bool testAutoSkip): Module(){}

        void flow() override{
            seq{
                cwhile(r1 < 20){
                    r1 = r1 + 1;
                    pick{
                        pif(r1 == 8){
                            a <<= 48;
                        }
                        pif(r1 == 16){
                            a <<= 24;
                        }
                        pickDef
                    }
                }
                rend <<= 48;
            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest38.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest38.vcd";


    class sim38 :public SimAutoInterface{
    public:

        testSimMod38* _md;

        sim38(testSimMod38* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              100,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                               prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void describeCon() override{
            int r1 = 1;
            //////before 8
            for (; r1 <= 8; r1++){
                testAndPrint("testR", ull(_md->r1), r1);
                conNextCycle(1);
            }
            testAndPrint("testR", ull(_md->r1), 8);
            conNextCycle(1);
            testAndPrint("testR", ull(_md->r1), 9);
            testAndPrint("testA", ull(_md->a), 48);
            conNextCycle(1);
            for(r1 = 10; r1 <= 16; r1++){
                testAndPrint("testR", ull(_md->r1), r1);
                conNextCycle(1);
            }
            testAndPrint("testR", ull(_md->r1), 16);
            conNextCycle(1);
            testAndPrint("testR", ull(_md->r1), 17);
            testAndPrint("testA", ull(_md->a), 24);
            conNextCycle(1);

            for (r1 = 18; r1 <= 20; r1++){
                conNextCycle(1);
            }
            testAndPrint("finalAns", ull(_md->rend), 0);
            conNextCycle(1);
            testAndPrint("finalAns", ull(_md->rend), 48);

        }

    };


    class Sim38TestEle: public AutoTestEle{
    public:
        explicit Sim38TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod38, 1);
            startModelKathryn();
            sim38 simulator( &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim38TestEle ele38(38);

    ///sim2 testCase2;




}