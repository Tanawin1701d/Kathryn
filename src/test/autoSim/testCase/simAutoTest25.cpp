//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod25: public Module{
    public:

        box(myType){
            initBox(myType);
            mReg(test1, 8);
            mReg(test2, 8);

            box(nexType){
                initBox(nexType);
                mReg(testn, 8);
            };

            mBox(n, nexType);
        };

        mBox(a, myType);
        mBox(b, myType);

        myType* ccccc;

        explicit testSimMod25(int x): Module(){}

        void flow() override{

            seq{
                a.test1   <<= 48;
                a.test2   <<= 50;
                a.n.testn <<= 99;
                b         <<= a;
                mBox(c, myType);
                ccccc = &c;
                c         = a;
            }


        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest25.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest25.vcd";


    class sim25 :public SimAutoInterface{
    public:

        testSimMod25* _md;

        sim25(testSimMod25* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void describeCon() override{

            conNextCycle(4);
            testAndPrint("check bundle assign b -> test1", ull(_md->b.test1),48);
            testAndPrint("check bundle assign b -> test2", ull(_md->b.test2),50);
            testAndPrint("check bundle assign b -> testn", ull(_md->b.n.testn),99);
            testAndPrint("check bundle assign c -> test1", ull(_md->ccccc->test1),48);
            testAndPrint("check bundle assign c -> test2", ull(_md->ccccc->test2),50);
            testAndPrint("check bundle assign c -> testn", ull(_md->ccccc->n.testn),99);
//            for(int i = 0; i < 48; i++){
//                ValRep testVal = NumConverter::createValRep(6, i);
//
//                testAndPrint("checkMem : " + std::to_string(i), _md->b.sv(), testVal);
//                conNextCycle(1);
//            }
        }

    };


    class Sim25TestEle: public AutoTestEle{
    public:
        explicit Sim25TestEle(int id): AutoTestEle(id){}
        void start() override{
            mMod(d, testSimMod25, 1);
            startModelKathryn();
            sim25 simulator((testSimMod25*) &d);
            simulator.simStart();
        }

    };

    Sim25TestEle ele25(25);
}