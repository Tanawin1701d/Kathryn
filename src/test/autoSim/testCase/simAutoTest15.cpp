//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod15: public Module{
    public:

        mReg(i, 6);
        mReg(k, 8);
        mReg(b, 6);
        mReg(c, 32);
        mMem(storage, 48, 8);


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

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest15.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest15.vcd";


    class sim15 :public SimAutoInterface{
    public:

        testSimMod15* _md;

        sim15(testSimMod15* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        int i = 0;
        void simAssert() override{


             incCycle(54);



             for ( int j = 0; j < 48; j++ ){
                sim{
                    ull testVal = i;
                    testAndPrint("check mem : " + std::to_string(i), (ull)_md->b, testVal);
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
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod15, 1);
            startModelKathryn();
            sim15 simulator((testSimMod15*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim15TestEle ele15(15);
}