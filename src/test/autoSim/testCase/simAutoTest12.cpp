//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod12: public Module{
    public:

        makeReg(a  , 67);
        makeReg(b  , 33);
        makeVal(BNK,  8, 128);
        makeVal(one,  1, 1);
        makeVal(ss,   8, 66);

        explicit testSimMod12(int x): Module(){}

        void flow() override{

            seq{
                b <<= BNK;
                b <<= b + one;
                a <<= b(7, 10);
                a <<= a << ss;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest12.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest12.vcd";


    class sim12 :public SimAutoInterface{
    public:

        testSimMod12* _md;

        sim12(testSimMod12* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(6);
            sim {
                ValRep testVal = NumConverter::cvtStrToValRep(67, 0b100, 0);
                testAndPrint("check shifting from slicing", _md->a.sv(), testVal);
            };
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


    class Sim12TestEle: public AutoTestEle{
    public:
        explicit Sim12TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod12, 1);
            sim12 simulator((testSimMod12*) &d);
            simulator.simStart();
        }

    };

    Sim12TestEle ele12(12);
}