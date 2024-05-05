//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod14: public Module{
    public:

        mReg(a, 32);
        mReg(b, 32);
        mReg(c, 32);

        explicit testSimMod14(int x): Module(){}

        void flow() override{

            seq{
                /** we want [3,6]*/
                a(3, 8)  <<= 3 + 16;
                b(9, 13) <<= 3;
                c <<= a(3, 7) + b(9, 13);

            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest14.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest14.vcd";


    class sim14 :public SimAutoInterface{
    public:

        testSimMod14* _md;

        sim14(testSimMod14* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(5);

            sim{
                ValRep testVal = NumConverter::createValRep(32, 6);
                testAndPrint("check expression on slicing", _md->c.sv(), testVal);
            };

//            sim {
//                ValRep testVal = NumConverter::cvtStrToValRep(67, 0b100, 0);
//                testAndPrint("check shifting from slicing", _md->a.sv(), testVal);
//            };
//            sim {
//                ValRep testVal = NumConverter::cvtStrToValRep(8, 2 + 6*2);
//                testAndPrint("check base line function", _md->a.sv(), testVal);
//            };
//            sim{
//                ValRep testVal = NumConverter::cvtStrToValRep(8, 48 - 6*2);
//                testAndPrint("check bascheck base line functione line function", _md->b.sv(), testVal);
//            };

        }

        void simDriven() override{
            incCycle(2);

//            sim {
//                _md->a.sv() = NumConverter::cvtStrToValRep(8,  2);
//                _md->b.sv() = NumConverter::cvtStrToValRep(8, 48);
//            };

        }

    };


    class Sim14TestEle: public AutoTestEle{
    public:
        explicit Sim14TestEle(int id): AutoTestEle(id){}
        void start() override{
            mMod(d, testSimMod14, 1);
            startModelKathryn();
            sim14 simulator((testSimMod14*) &d);
            simulator.simStart();
        }

    };

    Sim14TestEle ele14(14);
}