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

        makeReg(a, 8);
        makeReg(b, 8);
        makeVal(cond, 3, 2);
        makeVal(one, 8, 1);
        makeVal(two, 8, 2);
        makeVal(three, 8, 3);

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

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest11.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest11.vcd";


    class sim11 :public SimAutoInterface{
    public:

        testSimMod11* _md;

        sim11(testSimMod11* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
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
        void start() override{
            makeMod(d, testSimMod11, 1);
            sim11 simulator((testSimMod11*) &d);
            simulator.simStart();
        }

    };

    Sim11TestEle ele11(11);

    ///sim2 testCase2;

}