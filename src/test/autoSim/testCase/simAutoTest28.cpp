//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod28: public Module{
    public:

        mReg(a, 8);
        mReg(a2, 8);
        mVal(b, 8, 48);
        mReg(c, 1);
        mReg(d, 1);
        mWire(is, 1);

        explicit testSimMod28(int x): Module(){}

        void flow() override{

            cwhile(  c == 1 ){
                cif(a < 10){ intrStart(is); ////// it start at cif not inside cif
                    a = a+1;
                }celse{
                    a = a+2;
                };

                sif(a2 < 7){ intrStart(is); ////// it start at cif not inside sif
                    a2 = a2+1;
                }selif(a2 < 23){
                    a2 = a2+2;
                }
            }

            seq{
                syWait(5);
                par {
                    is = 1;
                    c  = 1;
                }
            }


        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest28.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest28.vcd";


    class sim28 :public SimAutoInterface{
    public:

        testSimMod28* _md;

        sim28(testSimMod28* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            conNextCycle(5);
            testAndPrint("check intr ifelse trigger c start", ull(_md->c),1);
            conNextCycle(1);
            testAndPrint("check intr par a start",  ull(_md->a ),1);
            testAndPrint("check intr par a2 start", ull(_md->a2),0);
            conNextCycle(1);
            testAndPrint("check intr par a start",  ull(_md->a ),1);
            testAndPrint("check intr par a2 start", ull(_md->a2),1);
            conNextCycle(1);
            testAndPrint("check intr par a start",  ull(_md->a ),2);
            testAndPrint("check intr par a2 start", ull(_md->a2),1);
            conNextCycle(1);
            testAndPrint("check intr par a start",  ull(_md->a ),2);
            testAndPrint("check intr par a2 start", ull(_md->a2),2);
            conNextCycle(1);
            testAndPrint("check intr par a start",  ull(_md->a ),3);
            testAndPrint("check intr par a2 start", ull(_md->a2),2);
            conNextCycle(1);
            testAndPrint("check intr par a start",  ull(_md->a ),3);
            testAndPrint("check intr par a2 start", ull(_md->a2),3);
        }

    };


    class Sim28TestEle: public AutoTestEle{
    public:
        explicit Sim28TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod28, 1);
            startModelKathryn();
            sim28 simulator((testSimMod28*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim28TestEle ele28(28);
}