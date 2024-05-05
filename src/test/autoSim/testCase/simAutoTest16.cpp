//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod16: public Module{
    public:

        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);

        mReg(result, 32);


        explicit testSimMod16(int x): Module(){}

        void flow() override{

            seq{
                a <<= 1;
                b(2) <<= 1;
                c(3) <<= 1;
                d(4) <<= 1;
                result   <<= g(d(0, 4), d(5,8),d(4),c,b,a);
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest16.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest16.vcd";


    class sim16 :public SimAutoInterface{
    public:

        testSimMod16* _md;

        sim16(testSimMod16* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


              incCycle(7);
//
            sim{
                ValRep testVal = NumConverter::createValRep(32, 0x01080401);
                testAndPrint("nest check with slice", _md->result.sv(), testVal);
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


    class Sim16TestEle: public AutoTestEle{
    public:
        explicit Sim16TestEle(int id): AutoTestEle(id){}
        void start() override{
            mMod(d, testSimMod16, 1);
            startModelKathryn();
            sim16 simulator((testSimMod16*) &d);
            simulator.simStart();
        }

    };

    Sim16TestEle ele16(16);
}