//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod61: public Module{
    public:

        mReg(a, 32);
        mReg(b, 32);

        explicit testSimMod61(int x){}

        void flow() override{
            ///// test priority of the zif block

            seq{
                a <<= 0;
                par{
                    SET_CLK_MODE2NEG_EDGE();
                    b <<= 48;
                    SET_CLK_MODE2DEF();
                    a <<= a + b;
                }
            }


        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest61.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest61.vcd";


    class sim61 :public SimAutoInterface{
    public:

        testSimMod61* _md;

        sim61(testSimMod61* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            conNextCycle(2);
            testAndPrint("a value updated from negEdge b:" + std::to_string(48)  , ull(_md->a), 48);
        }

    };


    class Sim61TestEle: public AutoTestEle{
    public:
        explicit Sim61TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod61, 1);
            startModelKathryn();
            sim61 simulator((testSimMod61*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim61TestEle ele61(61);
}