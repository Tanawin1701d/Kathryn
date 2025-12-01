//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod58: public Module{
    public:

        mReg(a, 32);

        explicit testSimMod58(int x){}

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

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest58.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest58.vcd";


    class sim58 :public SimAutoInterface{
    public:

        testSimMod58* _md;

        sim58(testSimMod58* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // // ////// skip first zync State
            // conNextCycle(1);
            // conEndCycle();
            for (int i = 2; i <= 8; i++){
                testAndPrint("a value " + std::to_string(i)  , ull(_md->a), i);
                conNextCycle(1);
                conEndCycle();
            }
            for (int i = 10; i <= 16; i+=2){
                testAndPrint("a value " + std::to_string(i)  , ull(_md->a), i);
                conNextCycle(1);
                conEndCycle();
            }
            testAndPrint("a value " + std::to_string(24)  , ull(_md->a), 24);
            conNextCycle(1);
            conEndCycle();
            testAndPrint("a value " + std::to_string(48)  , ull(_md->a), 48);
        }

    };


    class Sim58TestEle: public AutoTestEle{
    public:
        explicit Sim58TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod58, 1);
            startModelKathryn();
            sim58 simulator((testSimMod58*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim58TestEle ele58(58);
}