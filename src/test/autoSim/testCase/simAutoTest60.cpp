//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod60: public Module{
    public:

        mReg(a, 32);

        explicit testSimMod60(int x){}

        void flow() override{
            ///// test priority of the zif block

            zif (a < 16){
                a <<= a + 3;
                SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
                a <<= a + 2;
                SET_ASM_PRI_TO_AUTO();
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

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest60.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest60.vcd";


    class sim60 :public SimAutoInterface{
    public:

        testSimMod60* _md;

        sim60(testSimMod60* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{
            // for (int i = 2; i <= 40; i++){
            //     testAndPrint("a value " + std::to_string(i)  , ull(_md->a), 48);
            //     conNextCycle(1);
            //     conEndCycle();
            // }
        }

    };


    class Sim60TestEle: public AutoTestEle{
    public:
        explicit Sim60TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod60, 1);
            startModelKathryn();
            sim60 simulator((testSimMod60*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim60TestEle ele60(60);
}