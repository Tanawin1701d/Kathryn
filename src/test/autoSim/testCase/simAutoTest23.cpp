//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod23: public Module{
    public:
        mReg(a, 32);
        mReg(b, 32);
        mReg(c, 32);
        mReg(d, 32);

        explicit testSimMod23(int x): Module(){}

        void flow() override{

            pipWrap{
                /////// pipe block 0
                pipBlk{ a <<= a + 1;}
                /////// pipe block 1
                pipBlk{b <<= a;}
                /////// pipe block 2
                pipBlk{
                        c <<= b;
                        cif(c == 5){
                            syWait(6)
                        }
                      }
                /////// pipe block 3
                pipBlk{d <<= c;}
            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest23.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest23.vcd";


    class sim23 :public SimAutoInterface{
    public:

        testSimMod23* _md;

        sim23(testSimMod23* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                                _md(md)
        {}

        void describeCon() override{

                /*** start cycle*/
                for (int i = 0; i <= 8; i++){
                    testAndPrint("testPipVal: A", ull(_md->a), std::max(0,i  ));
                    testAndPrint("testPipVal: B", ull(_md->b), std::max(0,i-1));
                    testAndPrint("testPipVal: C", ull(_md->c), std::max(0,i-2));
                    testAndPrint("testPipVal: D", ull(_md->d), std::max(0,i-3));
                    conNextCycle(1);
                }
                /*** wait cycle*/
                conNextCycle(6);

                for (int i = 9; i < 12; i++){
                    testAndPrint("testPipAfterWaitVal: A", ull(_md->a), std::max(0,i  ));
                    testAndPrint("testPipAfterWaitVal: B", ull(_md->b), std::max(0,i-1));
                    testAndPrint("testPipAfterWaitVal: C", ull(_md->c), std::max(0,i-2));
                    testAndPrint("testPipAfterWaitVal: D", ull(_md->d), std::max(0,i-3));
                    conNextCycle(1);
                }



        }



    };


    class Sim23TestEle: public AutoTestEle{
    public:
        explicit Sim23TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod23, 1);
            startModelKathryn();
            sim23 simulator((testSimMod23*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim23TestEle ele23(23);
}