//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod67: public Module{
    public:
        mReg(a, 32);
        mReg(b, 32);
        mReg(r1, 32);
        mReg(r2, 32);
        mReg(res, 32);
        SyncMeta fetch{"fetch"};
        SyncMeta decode{"decode"};


        explicit testSimMod67(int x){}

        void flow() override{
            a.makeResetEvent();
            b.makeResetEvent();

            pip(fetch){ autoSync
                zync(decode){
                    a <<= 128;
                    b <<= 256;
                }
            }

            pip(decode){
                seq{
                    par{
                        sqrtInt(a, r1);
                        sqrtInt(b, r2);
                    }
                    res <<= r1 * r2;
                }
            }

            seq{
                syWait(3)
                par{decode.killSlave(false);}
            }

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest67.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest67.vcd";


    class sim67 :public SimAutoInterface{
    public:

        testSimMod67* _md;

        sim67(testSimMod67* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // ////// skip first zync State
            conNextCycle(10);
            testAndPrint("check r1  equal to " + std::to_string( 64), ull(_md->r1 ), 64 );
            testAndPrint("check r2  equal to " + std::to_string(128), ull(_md->r2 ), 128);
            testAndPrint("check res equal to " + std::to_string(  0), ull(_md->res), 0  );
            // for (int i = 1; i < 5; i++){
            //     conEndCycle();
            //     testAndPrint("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     testAndPrint("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     conNextCycle(1);
            // }
            // ///////////////////////////
            // conEndCycle();
            // testAndPrint(" holding part check a equal to " + std::to_string(4), ull(_md->a), 4);
            // testAndPrint(" holding part check b equal to " + std::to_string(4), ull(_md->b), 4);
            // conNextCycle(1);
            // for (int i = 5; i < 10; i++){
            //     conEndCycle();
            //     testAndPrint("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     testAndPrint("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     conNextCycle(1);
            // }

        }

    };


    class Sim67TestEle: public AutoTestEle{
    public:
        explicit Sim67TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod67, 1);
            startModelKathryn();
            sim67 simulator((testSimMod67*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim67TestEle ele67(67);
}
