//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod17: public Module{
    public:

        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        mReg(e, 8);
        mReg(f, 8);
        mReg(h, 8);


        mReg(result, 32);


        explicit testSimMod17(int x): Module(){}

        void flow() override{

            seq{
                a <<= 1;
                b <<= 1;
                g(d,
                  g(f(0,4),e(0,4)),
                  h)(8, 17) <<= 17 + (1 << 8);
            }

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest17.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest17.vcd";


    class sim17 :public SimAutoInterface{
    public:

        testSimMod17* _md;

        sim17(testSimMod17* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void simAssert() override{


              incCycle(5);
//
            sim{
                ValRep testVal = NumConverter::createValRep(8, 1);
                testAndPrint("test recur nest d", _md->d.sv(), testVal);
                testAndPrint("test recur nest e", _md->e.sv(), testVal);
                testAndPrint("test recur nest f", _md->f.sv(), testVal);
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
            incCycle(7);

//            sim {
//                _md->a.sv() = NumConverter::cvtStrToValRep(8,  2);
//                _md->b.sv() = NumConverter::cvtStrToValRep(8, 48);
//            };

        }

    };


    class Sim17TestEle: public AutoTestEle{
    public:
        explicit Sim17TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod17, 1);
            startModelKathryn();
            sim17 simulator((testSimMod17*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim17TestEle ele17(17);
}