//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod69: public Module{
    public:
        mReg (a, 32);
        mWire(myReset, 1);
        mWire(myStart, 1);


        explicit testSimMod69(int x){}

        void flow() override{

            cloop(bid, 20){ intrReset(myReset); intrStart(myStart);
                a <<= (bid);
            }


            seq{
                syWait(10);
                myReset = 1;
                syWait(2);
                myStart = 1;
            }



            // SyncMeta x;
            //
            // cwhile(true) zync(x)
            //         a <<= a;
            //
            // pip(x)
            // cloop(bid, 10)
            //     cloop(iter, 100)
            //         zif(iter == 99 && bid != 9)
            //             a <<= a >> 1;
        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest69.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest69.vcd";


    class sim69 :public SimAutoInterface{
    public:

        testSimMod69* _md;

        sim69(testSimMod69* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::toString(idx)+".vcd",
                                              prefix + "simAutoResult"+std::toString(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            testAndPrint("check a REG is not set to" + std::toString(0), ull(_md->a), 0);
            conNextCycle(1);
            testAndPrint("check a REG is not set to" + std::toString(0), ull(_md->a), 0);
            conNextCycle(1);
            for (int i = 1; i < 10; i++){
                testAndPrint("check a REG is not set to" + std::toString(i), ull(_md->a), i);
                conNextCycle(1);
            }

            for (int i = 0; i < 4; i++){
                testAndPrint("check a REG is not set to" + std::toString(i), ull(_md->a), 9);
                conNextCycle(1);
            }
            for (int i = 0; i < 5; i++){
                testAndPrint("check a REG is not set to" + std::toString(i), ull(_md->a), i);
                conNextCycle(1);
            }

            // conNextCycle(1);
            // conEndCycle();
            // testAndPrint("check a REG is not set to" + std::toString(0), ull(_md->a), 48);
            // testAndPrint("check b REG is not set to" + std::toString(48), ull(_md->b), 0);


            // ////// skip first zync State
            // conNextCycle(1);
            // for (int i = 1; i < 5; i++){
            //     conEndCycle();
            //     testAndPrint("check a equal to " + std::toString(i), ull(_md->a), i);
            //     testAndPrint("check b equal to " + std::toString(i-1), ull(_md->b), i-1);
            //     conNextCycle(1);
            // }
            // ///////////////////////////
            // conEndCycle();
            // testAndPrint(" holding part check a equal to " + std::toString(4), ull(_md->a), 4);
            // testAndPrint(" holding part check b equal to " + std::toString(4), ull(_md->b), 4);
            // conNextCycle(1);
            // for (int i = 5; i < 10; i++){
            //     conEndCycle();
            //     testAndPrint("check a equal to " + std::toString(i), ull(_md->a), i);
            //     testAndPrint("check b equal to " + std::toString(i-1), ull(_md->b), i-1);
            //     conNextCycle(1);
            // }

        }

    };


    class Sim69TestEle: public AutoTestEle{
    public:
        explicit Sim69TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod69, 1);
            startModelKathryn();
            sim69 simulator((testSimMod69*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim69TestEle ele69(69);
}
