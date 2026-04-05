//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod40: public Module{
    public:

        mReg (i, 6);
        mReg (k, 8);
        mWire(b, 6);
        mReg(c, 32);
        mMem(storage, 48, 8);


        explicit testSimMod40(int x): Module(){}

        void flow() override{

            i.markSv("myI");
            b.markSv("myB");
            storage.markSv("myMem");

            seq{
                i = 0;
                cwhile(i < 48){
                    par {
                        storage[i] = k;
                        i          = i + 1;
                        k          = k + 1;
                    }
                }
                i = 0;
                cwhile(i < 48){
                    par {
                        i = i + 1;
                        b = storage[i];
                    }
                }
            }

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest40.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest40.vcd";


    class sim40 :public SimAutoInterface{
    public:

        testSimMod40* _md;

        sim40(testSimMod40* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::toString(idx)+".vcd",
                                              prefix + "simAutoResult"+std::toString(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{
            conNextCycle(50);
            for (int i =1; i < 48; i++){
                testAndPrint("checkMem with new system : " + std::toString(i), ull(_md->storage.at(i)), i);
                conEndCycle();
                testAndPrint("checkMem wire assignment : " + std::toString(i), ull(_md->b), i);
                conNextCycle(1);
            }
//            for(int i = 0; i < 48; i++){
//                ValRep testVal = NumConverter::createValRep(6, i);
//
//                testAndPrint("checkMem : " + std::toString(i), _md->b.sv(), testVal);
//                conNextCycle(1);
//            }
        }

    };


    class Sim40TestEle: public AutoTestEle{
    public:
        explicit Sim40TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod40, 1);
            startModelKathryn();
            sim40 simulator((testSimMod40*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim40TestEle ele40(40);
}