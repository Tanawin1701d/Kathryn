//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod51: public Module{
    public:
        mReg(a, 32);
        mReg(b, 32);
        mReg(c, 32);
        mWire(purpose, 32);
        SyncMeta fetch{"fetch"};
        SyncMeta decode{"decode"};


        explicit testSimMod51(int x){}

        void flow() override{
            a.makeResetEvent();

            pip(fetch){ autoSync
                zync(decode){
                    a <<= a + 1;
                    purpose = a + 1;
                }
            }

            pip(decode){
                b <<= a;
            }

            seq{
                c <<= c + 1;
                c <<= c + 1;
                c <<= c + 1;
                c <<= c + 1;
                fetch.holdSlave();
            }

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest51.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest51.vcd";


    class sim51 :public SimAutoInterface{
    public:

        testSimMod51* _md;

        sim51(testSimMod51* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // ////// skip first zync State
            conNextCycle(1);
            for (int i = 1; i < 5; i++){
                conEndCycle();
                testAndPrint("check a equal to " + std::to_string(i), ull(_md->a), i);
                testAndPrint("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
                conNextCycle(1);
            }
            ///////////////////////////
            conEndCycle();
            testAndPrint(" holding part check a equal to " + std::to_string(4), ull(_md->a), 4);
            testAndPrint(" holding part check b equal to " + std::to_string(4), ull(_md->b), 4);
            conNextCycle(1);
            for (int i = 5; i < 10; i++){
                conEndCycle();
                testAndPrint("check a equal to " + std::to_string(i), ull(_md->a), i);
                testAndPrint("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
                conNextCycle(1);
            }

        }

    };


    class Sim51TestEle: public AutoTestEle{
    public:
        explicit Sim51TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod51, 1);
            startModelKathryn();
            sim51 simulator((testSimMod51*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim51TestEle ele51(51);
}