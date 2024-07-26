//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod37: public Module{
    public:
        mReg(a, 128);
        mReg(b, 128);

        mReg(r0, 128);
        mReg(r1, 128);
        mReg(r2, 128);
        mReg(r3, 128);

        explicit testSimMod37(int x): Module(){}

        void flow() override{
            seq{
                a(64, 72) <<= 1;
                b(64, 72) <<= 3;
                r0 <<= a & b;
                r1 <<= a | b;
                r2 <<= a ^ b;
                r3 <<= !a;

            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest37.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest37.vcd";


    class sim37 :public SimAutoInterface{
    public:

        testSimMod37* _md;

        sim37(testSimMod37* md, int idx, const std::string& prefix):SimAutoInterface(idx,
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


    class Sim37TestEle: public AutoTestEle{
    public:
        explicit Sim37TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod37, 1);
            startModelKathryn();
            sim37 simulator((testSimMod37*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim37TestEle ele37(37);
}