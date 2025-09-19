//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod53: public Module{
    public:
        SlotMeta meta{{"valid", "srcIdx0", "srcIdx1"},
            {1,32,32}
        };
        SlotMeta meta2{{"srcIdx3", "srcIdx4"},
                        {32, 32}
        };
        RegSlot rs {meta };
        RegSlot rs2{meta + meta2};
        mReg(idx, 4);


        explicit testSimMod53(int x){}

        void flow() override{
            ///// dynamic assign and slot assign test
            seq{
                rs("srcIdx0") <<= 24;
                rs("srcIdx1") <<= 48;
                idx <<= 0;
                rs(1, 3)[idx] <<= 26;
                idx <<= idx + 1;
                rs(1, 3)[idx] <<= 50;
                rs2 <<= rs;


            }




        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest53.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest53.vcd";


    class sim53 :public SimAutoInterface{
    public:

        testSimMod53* _md;

        sim53(testSimMod53* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
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
            conNextCycle(2);
            conEndCycle();
            testAndPrint("check regSlot assign 26 ", ull(_md->rs("srcIdx0")), 26);
            testAndPrint("check regSlot assign 48 ", ull(_md->rs("srcIdx1")), 48);
            conNextCycle(2);
            conEndCycle();
            testAndPrint("check regSlot assign 26 ", ull(_md->rs("srcIdx0")), 26);
            testAndPrint("check regSlot assign 50 ", ull(_md->rs("srcIdx1")), 50);
            conNextCycle(1);
            conEndCycle();
            testAndPrint("check whole regSlot assign 26 ", ull(_md->rs2("srcIdx0")), 26);
            testAndPrint("check whole regSlot assign 50 ", ull(_md->rs2("srcIdx1")), 50);
            testAndPrint("check whole regSlot other field (srcIdx3)", ull(_md->rs2("srcIdx3")), 0);
            testAndPrint("check whole regSlot other field (srcIdx4)", ull(_md->rs2("srcIdx4")), 0);

            // testAndPrint("check wireSlot assign 24 ", ull(_md->ws("srcIdx0")), 24);
            // testAndPrint("check wireSlot assign 48 ", ull(_md->ws("srcIdx1")), 48);
            // conNextCycle(2);
            // conEndCycle();
            //
            // testAndPrint("check wireSlot Retrieve 24 ", ull(_md->src0get), 24);
            //
            // conNextCycle(1);
            // conEndCycle();
            // testAndPrint("check wireSlot No retrieve ", ull(_md->src0get), 0);
            //
            // conNextCycle(1);
            // conEndCycle();
            // testAndPrint("check wireSlot No 48 ", ull(_md->src0get), 48);

        }

    };


    class Sim53TestEle: public AutoTestEle{
    public:
        explicit Sim53TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod53, 1);
            startModelKathryn();
            sim53 simulator((testSimMod53*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim53TestEle ele53(-1);
}