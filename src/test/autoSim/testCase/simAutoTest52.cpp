//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod52: public Module{
    public:
        SlotMeta meta{{"valid", "srcIdx0", "srcIdx1"},
            {1,32,32}
        };
        RegSlot rs{meta};
        WireSlot ws{{"srcIdx0", "srcIdx1"},
            {32,32}};
        mWire(src0get, 32);
        mReg(idx, 4);


        explicit testSimMod52(int x){}

        void flow() override{

            seq{
                rs("srcIdx0") <<= 24;
                rs("srcIdx1") <<= 48;
                par{
                    zif(rs("srcIdx0") == 24){
                        ws = rs;
                    }
                }
                idx <<= 0;
                src0get  = rs(1, 3)[idx].v();
                idx <<= idx + 1;
                src0get = rs(1, 3)[idx].v();

            }




        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest52.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest52.vcd";


    class sim52 :public SimAutoInterface{
    public:

        testSimMod52* _md;

        sim52(testSimMod52* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // // ////// skip first zync State
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


    class Sim52TestEle: public AutoTestEle{
    public:
        explicit Sim52TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod52, 1);
            startModelKathryn();
            sim52 simulator((testSimMod52*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim52TestEle ele52(-1);
}