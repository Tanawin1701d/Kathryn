//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod20: public Module{
    public:
        mReg(a, 32);
        mWire(result, 1);

        explicit testSimMod20(int x): Module(){}

        void flow() override{

            seq {
                par{
                    seq {
                        scWait(a == 48);
                        result = 1;
                    }
                    cwhile(a <= 50){
                        a <<= a + 1;
                    }
                }


            }

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest20.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest20.vcd";


    class sim20 :public SimAutoInterface{
    public:

        testSimMod20* _md;

        sim20(testSimMod20* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              300,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void describeCon() override{
            for(int i = 0; i <= 48; i++){
                if ((i % 10) == 0) {
                    conEndCycle();
                    testAndPrint("testResult is waiting", ull(_md->result), 0);
                }
                conNextCycle(1);
            }
            conEndCycle();
            testAndPrint("testResult is waiting set", ull(_md->result), 1);

        }

    };


    class Sim20TestEle: public AutoTestEle{
    public:
        explicit Sim20TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod20, 1);
            startModelKathryn();
            sim20 simulator((testSimMod20*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim20TestEle ele20(20);
}