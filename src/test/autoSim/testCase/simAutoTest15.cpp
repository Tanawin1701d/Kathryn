//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod15: public Module{
    public:

        makeReg(i, 6);
        makeReg(k, 8);
        makeReg(b, 6);
        makeReg(c, 32);
        makeMem(storage, 48, 8);


        explicit testSimMod15(int x): Module(){}

        void flow() override{

            seq{
                i <<= 0;
                cwhile(i < 46){
                    /////// max in this scope is 47
                    par {
                        storage[i] <<= k;
                        i <<= i + 1;
                        k <<= k + 1;
                    }
                }
                i <<= 0;
                i <<= 0;
                cwhile(i < 46){
                    par {
                        i <<= i + 1;
                        b <<= storage[i];
                    }
                }
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest15.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest15.vcd";


    class sim15 :public SimAutoInterface{
    public:

        testSimMod15* _md;

        sim15(testSimMod15* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


//            incCycle(6);
//
//            sim{
//                ValRep testVal = NumConverter::cvtStrToValRep(32, 6);
//                testAndPrint("check expression on slicing", _md->c.sv(), testVal);
//            };

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


    class Sim15TestEle: public AutoTestEle{
    public:
        explicit Sim15TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod15, 1);
            sim15 simulator((testSimMod15*) &d);
            simulator.simStart();
        }

    };

    Sim15TestEle ele15(15);
}