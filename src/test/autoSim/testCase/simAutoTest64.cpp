//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod64: public Module{
    public:


        mReg(  a, 32);
        mReg(res, 32);

        explicit testSimMod64(int x){}

        void flow() override{
            ///// test priority of the zif block

            seq{
                a <<= 40;
                res <<= sqrtInt(a);
            }


        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest64.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest64.vcd";


    class sim64 :public SimAutoInterface{
    public:

        testSimMod64* _md;

        sim64(testSimMod64* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{
            conNextCycle(8);
            testAndPrint("sqrt Val", ull(_md->res), 6);
        }

    };


    class Sim64TestEle: public AutoTestEle{
    public:
        explicit Sim64TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod64, 1);
            startModelKathryn();
            sim64 simulator((testSimMod64*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim64TestEle ele64(-1);
}
