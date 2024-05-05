//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod27: public Module{
    public:

        mReg(a, 8);
        mVal(b, 8, 48);
        mReg(c, 1);
        mReg(d, 1);
        mWire(is, 1);

        explicit testSimMod27(int x): Module(){}

        void flow() override{

            cwhile( (c == 1) & (a < 48) ){
                par {
                    intrStart(is);
                    c = 1;
                    a <<= a + 1;
                    seq{
                      d = 1;
                    }
                }
            }

            seq{
                syWait(5);
                is = 1;
            }


        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest27.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest27.vcd";


    class sim27 :public SimAutoInterface{
    public:

        testSimMod27* _md;

        sim27(testSimMod27* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void describeCon() override{

            conNextCycle(6);
            testAndPrint("check intr par c start", ull(_md->c),1);
            testAndPrint("check intr par d start", ull(_md->d),1);
            conNextCycle(1);
            testAndPrint("check intr par a start", ull(_md->a),1);
            testAndPrint("check intr par c stay", ull(_md->c),1);
            conNextCycle(1);
            testAndPrint("check intr par a start", ull(_md->a),2);


        }

    };


    class Sim27TestEle: public AutoTestEle{
    public:
        explicit Sim27TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod27, 1);
            startModelKathryn();
            sim27 simulator((testSimMod27*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim27TestEle ele27(27);
}