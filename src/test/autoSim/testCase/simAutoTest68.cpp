//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod68: public Module{
    public:
        mReg (a, 32);
        mWire(b, 32);


        explicit testSimMod68(int x){}

        void flow() override{

            par{
                a <<= 48;
                b =   48;
            }




        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest68.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest68.vcd";


    class sim68 :public SimAutoInterface{
    public:

        testSimMod68* _md;

        sim68(testSimMod68* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            conEndCycle();
            testAndPrint("check a REG is not set to" + std::to_string(0), ull(_md->a), 0);
            testAndPrint("check b REG is not set to" + std::to_string(48), ull(_md->b), 48);
            conNextCycle(1);
            conEndCycle();
            testAndPrint("check a REG is not set to" + std::to_string(0), ull(_md->a), 48);
            testAndPrint("check b REG is not set to" + std::to_string(48), ull(_md->b), 0);


            // ////// skip first zync State
            // conNextCycle(1);
            // for (int i = 1; i < 5; i++){
            //     conEndCycle();
            //     testAndPrint("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     testAndPrint("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     conNextCycle(1);
            // }
            // ///////////////////////////
            // conEndCycle();
            // testAndPrint(" holding part check a equal to " + std::to_string(4), ull(_md->a), 4);
            // testAndPrint(" holding part check b equal to " + std::to_string(4), ull(_md->b), 4);
            // conNextCycle(1);
            // for (int i = 5; i < 10; i++){
            //     conEndCycle();
            //     testAndPrint("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     testAndPrint("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     conNextCycle(1);
            // }

        }

    };


    class Sim68TestEle: public AutoTestEle{
    public:
        explicit Sim68TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod68, 1);
            startModelKathryn();
            sim68 simulator((testSimMod68*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim68TestEle ele68(68);
}
