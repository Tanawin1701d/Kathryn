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
            conNextCycle(1);
            conEndCycle();
            testAndPrint("check regSlot assign 24 ", ull(_md->rs("srcIdx0")), 24);
            testAndPrint("check regSlot assign 0 ", ull(_md->rs("srcIdx1")), 0);
            conNextCycle(1);
            conEndCycle();
            testAndPrint("check regSlot assign 24 ", ull(_md->rs("srcIdx0")), 24);
            testAndPrint("check regSlot assign 48 ", ull(_md->rs("srcIdx1")), 48);
            testAndPrint("check wireSlot assign 24 ", ull(_md->ws("srcIdx0")), 24);
            testAndPrint("check wireSlot assign 48 ", ull(_md->ws("srcIdx1")), 48);
            conNextCycle(2);
            conEndCycle();

            testAndPrint("check wireSlot Retrieve 24 ", ull(_md->src0get), 24);

            conNextCycle(1);
            conEndCycle();
            testAndPrint("check wireSlot No retrieve ", ull(_md->src0get), 0);

            conNextCycle(1);
            conEndCycle();
            testAndPrint("check wireSlot No 48 ", ull(_md->src0get), 48);

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

    Sim52TestEle ele52(52);
}