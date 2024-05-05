//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod9: public Module{
    public:
        bool testAutoSkip = false;

        mVal(bnk, 32, 48);
        mVal(akb, 32, 48);
        mVal(endConst, 112, 0b1111111111111111);
        mReg(cnt, 32);
        mReg(frd, 32);
        mReg(end, 3);

        mVal(one, 32,  1);


        explicit testSimMod9(bool testAutoSkip): Module(){}

        void flow() override{

            seq {
                par {
                    swhile(cnt < bnk) {
                        cnt <<= cnt + one;
                    }
                    frd <<= akb;

                }
                end <<= endConst;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest9.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest9.vcd";


    class sim9 :public SimAutoInterface{
    public:

        testSimMod9* _md;

        sim9(testSimMod9* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(101);

            sim {
                ValRep testVal = NumConverter::createValRep(3, 0b111);
                testAndPrint("check end loop with synNode", _md->end.sv(), testVal);
            };

        }

        void simDriven() override{
            incCycle(2);

            sim {
                //_md->iv.sv() = NumConverter::cvtStrToValRep(8, 48);
            };
            incCycle(1);
        }

    };


    class Sim9TestEle: public AutoTestEle{
    public:
        explicit Sim9TestEle(int id): AutoTestEle(id){}
        void start() override{
            mMod(d, testSimMod9, 1);
            startModelKathryn();
            sim9 simulator((testSimMod9*) &d);
            simulator.simStart();
        }

    };

    Sim9TestEle ele9(9);

    ///sim2 testCase2;




}