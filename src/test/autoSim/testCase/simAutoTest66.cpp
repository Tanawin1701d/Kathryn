//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod66: public Module{
    public:


        mReg(  a, 32);
        mReg(  b, 32);
        mReg(r1, 32);
        mReg(r2, 32);
        mReg(res, 32);
        mWire(fin, 1);


        explicit testSimMod66(int x){}

        void flow() override{

            a.makeResetEvent(1);
            b.makeResetEvent(1);

            ///// test priority of the zif block
            pipStream{
                par{
                    a <<= a * 2;
                    b <<= b * 2;
                }
                seq{
                    par{
                        sqrtInt(a, r1);
                        sqrtInt(b, r2);
                    }
                    par{
                        res <<= r1 * r2;
                        fin = 1;
                    }
                }

            }
        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest66.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest66.vcd";


    class sim66 :public SimAutoInterface{
    public:

        testSimMod66* _md;

        sim66(testSimMod66* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

        }

    };


    class Sim66TestEle: public AutoTestEle{
    public:
        explicit Sim66TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod66, 1);
            startModelKathryn();
            sim66 simulator((testSimMod66*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim66TestEle ele66(-1);
}
