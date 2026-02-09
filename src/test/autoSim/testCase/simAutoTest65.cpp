//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod65: public Module{
    public:


        mReg(  a, 32);
        mReg(  b, 32);
        mReg(  c, 32);
        mReg(  d, 32);
        mReg(res, 32);

        explicit testSimMod65(int x){}

        void flow() override{
            ///// test priority of the zif block
            pipStream{
                seq{
                    a <<= a + 1;
                    syWait(3);
                    b <<= a;
                }

                seq{
                    c <<= b;
                    syWait(6);
                    d <<= c;
                }
            }
        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest65.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest65.vcd";


    class sim65 :public SimAutoInterface{
    public:

        testSimMod65* _md;

        sim65(testSimMod65* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{
            conNextCycle(2);
            testAndPrint("a", ull(_md->a), 1);
            testAndPrint("b", ull(_md->b), 0);
            testAndPrint("c", ull(_md->c), 0);
            testAndPrint("d", ull(_md->d), 0);
            conNextCycle(4);
            testAndPrint("a", ull(_md->a), 1);
            testAndPrint("b", ull(_md->b), 1);
            testAndPrint("c", ull(_md->c), 0);
            testAndPrint("d", ull(_md->d), 0);
            conNextCycle(1);
            testAndPrint("a", ull(_md->a), 2);
            testAndPrint("b", ull(_md->b), 1);
            testAndPrint("c", ull(_md->c), 1);
            testAndPrint("d", ull(_md->d), 0);
            conNextCycle(4);
            testAndPrint("a", ull(_md->a), 2);
            testAndPrint("b", ull(_md->b), 2);
            testAndPrint("c", ull(_md->c), 1);
            testAndPrint("d", ull(_md->d), 0);
            conNextCycle(4);
            testAndPrint("a", ull(_md->a), 3);
            testAndPrint("b", ull(_md->b), 2);
            testAndPrint("c", ull(_md->c), 2);
            testAndPrint("d", ull(_md->d), 1);
            conNextCycle(4);
            testAndPrint("a", ull(_md->a), 3);
            testAndPrint("b", ull(_md->b), 3);
            testAndPrint("c", ull(_md->c), 2);
            testAndPrint("d", ull(_md->d), 1);
            conNextCycle(4);
            testAndPrint("a", ull(_md->a), 4);
            testAndPrint("b", ull(_md->b), 3);
            testAndPrint("c", ull(_md->c), 3);
            testAndPrint("d", ull(_md->d), 2);
        }

    };


    class Sim65TestEle: public AutoTestEle{
    public:
        explicit Sim65TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod65, 1);
            startModelKathryn();
            sim65 simulator((testSimMod65*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim65TestEle ele65(65);
}
