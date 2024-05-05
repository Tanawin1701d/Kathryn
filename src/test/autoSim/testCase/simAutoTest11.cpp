//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod11: public Module{
    public:
        bool testAutoSkip = false;

        mReg(a, 8);
        mReg(b, 8);
        mVal(cond, 3, 2);
        mVal(one, 8, 1);
        mVal(two, 8, 2);
        mVal(three, 8, 3);

        explicit testSimMod11(bool testAutoSkip): Module(){}

        void flow() override{

            cwhile(cond){
                zif(a > b){
                    a <<= a + one;
                }
                zelif(a < b){
                    a <<= a + two;
                    zif(a > b){
                        b <<= b - one;
                    }zelse{
                        b <<= b - two;
                    }
                }
            }

        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest11.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest11.vcd";


    class sim11 :public SimAutoInterface{
    public:

        testSimMod11* _md;

        sim11(testSimMod11* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(3);
            incCycle(5);
            sim {
                ValRep testVal = NumConverter::createValRep(8, 2 + 6 * 2);
                testAndPrint("check base line function", _md->a.sv(), testVal);
            };
            sim{
                ValRep testVal = NumConverter::createValRep(8, 48 - 6 * 2);
                testAndPrint("check bascheck base line functione line function", _md->b.sv(), testVal);
            };

        }

        void simDriven() override{
            incCycle(2);

            sim {
                _md->a.sv() = NumConverter::createValRep(8, 2);
                _md->b.sv() = NumConverter::createValRep(8, 48);
            };

        }

    };


    class Sim11TestEle: public AutoTestEle{
    public:
        explicit Sim11TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod11, 1);
            startModelKathryn();
            sim11 simulator((testSimMod11*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim11TestEle ele11(11);

    ///sim2 testCase2;

}