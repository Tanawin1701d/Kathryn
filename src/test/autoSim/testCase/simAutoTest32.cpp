//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod32: public Module{
    public:
        mReg(a, 32);
        mReg(c, 32);
        mWire(x, 6);

        explicit testSimMod32(int x): Module(){}

        void flow() override{

            seq{
                a <<= 1;
                cdowhile(c <= 5){
                    par {
                        x = c;
                        c <<= c + 1;
                    }
                }
            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest32.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest32.vcd";


    class sim32 :public SimAutoInterface{
    public:

        testSimMod32* _md;

        sim32(testSimMod32* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                                _md(md)
        {}

        void describeCon() override{
            for (int i = 0; i < 2; i++){
                conEndCycle();
                testAndPrint("dowhilePre", ull(_md->x),0);
                conNextCycle(1);
            }
            for(int i = 1; i <= 6; i++){
                conEndCycle();
                testAndPrint("dowhileVal", ull(_md->x),i);
                conNextCycle(1);
            }
            for (int i = 0; i < 2; i++){
                conEndCycle();
                testAndPrint("dowhilePost", ull(_md->x),0);
                conNextCycle(1);
            }
        }
    };


    class Sim32TestEle: public AutoTestEle{
    public:
        explicit Sim32TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod32, 1);
            startModelKathryn();
            sim32 simulator((testSimMod32*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim32TestEle ele32(32);
}