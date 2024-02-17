//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod13: public Module{
    public:

        makeReg(a  , 67);
        makeReg(a2  , 67);
        makeReg(a3  , 67);
        makeReg(b  , 33);
        makeReg(b2  ,33);
        makeReg(b3 , 33);
        makeVal(BNK,  8, 138);
        makeVal(one,  1, 1);
        makeVal(five, 67, 5);
        makeVal(ss,   8, 66);
        makeVal(ten, 67, 10);

        explicit testSimMod13(int x): Module(){}

        void flow() override{

            seq{
                a2 <<= BNK;
                a2 <<= BNK;
                cwhile(a < a2){
                    a <<= a +  one;

                    cif(a >= ten) {
                        b <<= b + one;
                    }celif(a >= five){
                        b3 <<= b3 + one;
                    }celse{
                        b2 <<= b2 + one;
                    }


                }
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest13.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest13.vcd";


    class sim13 :public SimAutoInterface{
    public:

        testSimMod13* _md;

        sim13(testSimMod13* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(6);
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


    class Sim13TestEle: public AutoTestEle{
    public:
        explicit Sim13TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod13, 1);
            sim13 simulator((testSimMod13*) &d);
            simulator.simStart();
        }

    };

    Sim13TestEle ele13(-1);
}