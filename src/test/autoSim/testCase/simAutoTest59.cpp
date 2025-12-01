//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod59: public Module{
    public:

        mReg(a, 32);

        explicit testSimMod59(int x){}

        void flow() override{
            ///// test priority of the zif block

            zif (a < 16){
                a <<= a + 3;
                a <<= a + 2;
                zif (a < 8){
                    a <<= a + 1;
                }
            }zelif(a < 32){
                zif (a < 24){
                    a <<= 24;
                }zelse{
                    a <<= 48;
                }
            }
            a <<= 48;
        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest59.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest59.vcd";


    class sim59 :public SimAutoInterface{
    public:

        testSimMod59* _md;

        sim59(testSimMod59* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{
            for (int i = 2; i <= 40; i++){
                testAndPrint("a value " + std::to_string(i)  , ull(_md->a), 48);
                conNextCycle(1);
                conEndCycle();
            }
        }

    };


    class Sim59TestEle: public AutoTestEle{
    public:
        explicit Sim59TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod59, 1);
            startModelKathryn();
            sim59 simulator((testSimMod59*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim59TestEle ele59(59);
}