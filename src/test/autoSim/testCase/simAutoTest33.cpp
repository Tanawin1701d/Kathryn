//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod33: public Module{
    public:
        mReg(a, 32);
        mReg(c, 32);
        mWire(x, 6);

        explicit testSimMod33(int x): Module(){}

        void flow() override{
            seq{
                a <<= 1;
                cdowhile(c <= 5){
                    par {
                        x = c;
                        c <<= c + 1;
                        /////sbreakCon(c == 2);
                    }
                }
            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest33.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest33.vcd";


    class sim33 :public SimAutoInterface{
    public:

        testSimMod33* _md;

        sim33(testSimMod33* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                                _md(md)
        {}

        void describeCon() override{
            for (int i = 0; i < 2; i++){
                conEndCycle();
                testAndPrint("dowhilePre", ull(_md->x),0);
                conNextCycle(1);
            }
            for(int i = 1; i <= 6; i++){
                conEndCycle();
                testAndPrint("dowhileVal", ull(_md->x),i);
                conNextCycle(1);
            }
            for (int i = 0; i < 2; i++){
                conEndCycle();
                testAndPrint("dowhilePost", ull(_md->x),0);
                conNextCycle(1);
            }
        }
    };


    class Sim33TestEle: public AutoTestEle{
    public:
        explicit Sim33TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod33, 1);
            startModelKathryn();
            sim33 simulator((testSimMod33*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim33TestEle ele33(33);
}