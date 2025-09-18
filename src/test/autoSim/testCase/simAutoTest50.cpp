//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod50: public Module{
    public:
        mReg(a, 32);
        mReg(b, 32);
        mReg(c, 32);
        SyncMeta fetch{"fetch"};
        SyncMeta decode{"decode"};


        explicit testSimMod50(int x){}

        void flow() override{
            a.makeResetEvent();

            pip(fetch){ autoStart
                zync(decode){
                    a <<= a + 1;
                }
            }

            pip(decode){
                b <<= a;
            }

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest50.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest50.vcd";


    class sim50 :public SimAutoInterface{
    public:

        testSimMod50* _md;

        sim50(testSimMod50* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            ////// skip first zync State
            conNextCycle(1);
            for (int i = 1; i < 5; i++){
                conEndCycle();
                testAndPrint("check a equal to " + std::to_string(i), ull(_md->a), i);
                testAndPrint("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
                conNextCycle(1);
            }

        }

    };


    class Sim50TestEle: public AutoTestEle{
    public:
        explicit Sim50TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod50, 1);
            startModelKathryn();
            sim50 simulator((testSimMod50*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim50TestEle ele50(50);
}