//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod26: public Module{
    public:

        mReg(a, 8);
        mVal(b, 8, 48);
        mReg(c, 1);
        mWire(is, 1);

        explicit testSimMod26(int x): Module(){}

        void flow() override{

            cwhile( (c == 1) & (a < 48) ){
                seq {
                    intrStart(is);
                    c = 1;
                    a <<= a + 1;
                }
            }

            seq{
                syWait(5);
                is = 1;
            }


        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest26.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest26.vcd";


    class sim26 :public SimAutoInterface{
    public:

        testSimMod26* _md;

        sim26(testSimMod26* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void describeCon() override{

            conNextCycle(6);
            testAndPrint("check intr seq c start", ull(_md->c),1);
            conNextCycle(2);
            testAndPrint("check intr seq a start", ull(_md->a),1);
            conNextCycle(2);
            testAndPrint("check intr a start", ull(_md->a),2);


        }

    };


    class Sim26TestEle: public AutoTestEle{
    public:
        explicit Sim26TestEle(int id): AutoTestEle(id){}
        void start() override{
            mMod(d, testSimMod26, 1);
            startModelKathryn();
            sim26 simulator((testSimMod26*) &d);
            simulator.simStart();
        }

    };

    Sim26TestEle ele26(26);
}