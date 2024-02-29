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
                cwhile(i < 47){
                    /////// max in this scope is 47
                    par {
                        storage[i] <<= k;
                        i <<= i + 1;
                        k <<= k + 1;
                    }
                }
                i <<= 0;
                i <<= 0;
                cwhile(i < 47){
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

        int i = 0;
        void simAssert() override{


             incCycle(54);



             for ( int j = 0; j < 48; j++ ){
                sim{
                    ValRep testVal = NumConverter::createValRep(6, i);
                    testAndPrint("check mem : " + std::to_string(i), _md->b.sv(), testVal);
                    i++;
                };
                incCycle(1);
             }

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