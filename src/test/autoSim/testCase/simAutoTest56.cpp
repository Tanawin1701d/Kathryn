//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod56: public Module{
    public:
        SlotMeta meta{{"valid", "srcIdx0", "srcIdx1"},
            {1,32,32}
        };
        RegSlot rs{meta};
        WireSlot ws{{"srcIdx0", "srcIdx1"},
            {32,32}};

        explicit testSimMod56(int x){}

        void flow() override{

            seq{
                rs("srcIdx0") <<= 24;
                rs("srcIdx1") <<= 48;
                par{
                    ws = rs;
                    zif(rs("srcIdx0") == 24){
                        ws("srcIdx0") = 26;
                    }
                }
            }




        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest56.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest56.vcd";


    class sim56 :public SimAutoInterface{
    public:

        testSimMod56* _md;

        sim56(testSimMod56* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // // ////// skip first zync State
            std::cout << TC_BLUE << "test priority test" << TC_DEF << std::endl;
            conNextCycle(2);
            conEndCycle();
            testAndPrint("check wire assign 26 ", ull(_md->ws("srcIdx0")), 26);
            testAndPrint("check wire assign 48 ", ull(_md->ws("srcIdx1")), 48);
        }

    };


    class Sim56TestEle: public AutoTestEle{
    public:
        explicit Sim56TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod56, 1);
            startModelKathryn();
            sim56 simulator((testSimMod56*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim56TestEle ele56(56);
}