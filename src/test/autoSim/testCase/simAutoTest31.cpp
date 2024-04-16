//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod31: public Module{
    public:
        makeReg(a, 32);
        makeReg(b, 32);
        makeReg(c, 32);
        makeReg(d, 32);
        makeWire(is, 1);
        makeWire(freez , 1);
        makeWire(freez2, 1);

        explicit testSimMod31(int x): Module(){}

        void flow() override{

            pipWrap{
                /////// pipe block 0
                pipBlk{
                    intrStart(is);
                    intrReset(is);

                    cif(a < 5){
                        a = a + 1;
                    }celse{
                        cwhile(true){
                            seq {
                                freez  = 1;
                            }
                        }
                    }
                }
                /////// pipe block 1
                pipBlk{
                    b = b + 1;
                }
            }


            seq{
                syWait(10);
                par{
                    is = 1;
                    a  = 0;
                }
            }
        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest31.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest31.vcd";


    class sim31 :public SimAutoInterface{
    public:

        testSimMod31* _md;

        sim31(testSimMod31* md):SimAutoInterface(31,
                                                 300,
                                                 vcdPath,
                                                 profilePath),
                                _md(md)
        {}

        void describeCon() override{

//            /*** start cycle*/
              for (int i = 1; i <= 5; i++){
                  testAndPrint("testPipVal before resetAndRestart and getStuck", ull(_md->a), i);
                  conEndCycle();
                  testAndPrint("testPipVal before freez", ull(_md->freez), 0);
                  conNextCycle(1);
              }
              for (int i = 0; i < 6; i++){
                  conEndCycle();
                  testAndPrint("testPipVal freez", ull(_md->freez), 1);
                  conNextCycle(1);
              }

            for (int i = 1; i <= 5; i++){
                testAndPrint("testPipVal after resetAndRestart: a", ull(_md->a), i);
                conEndCycle();
                testAndPrint("testPipVal after freez", ull(_md->freez), 0);
                conNextCycle(1);
            }
//            for (int i = 0; i <= 8; i++){
//                testAndPrint("testPipVal: A", ull(_md->a), std::max(0,i  ));
//                testAndPrint("testPipVal: B", ull(_md->b), std::max(0,i-1));
//                testAndPrint("testPipVal: C", ull(_md->c), std::max(0,i-2));
//                testAndPrint("testPipVal: D", ull(_md->d), std::max(0,i-3));
//                conNextCycle(1);
//            }
//            /*** wait cycle*/
//            conNextCycle(6);
//
//            for (int i = 9; i < 12; i++){
//                testAndPrint("testPipAfterWaitVal: A", ull(_md->a), std::max(0,i  ));
//                testAndPrint("testPipAfterWaitVal: B", ull(_md->b), std::max(0,i-1));
//                testAndPrint("testPipAfterWaitVal: C", ull(_md->c), std::max(0,i-2));
//                testAndPrint("testPipAfterWaitVal: D", ull(_md->d), std::max(0,i-3));
//                conNextCycle(1);
//            }



        }



    };


    class Sim31TestEle: public AutoTestEle{
    public:
        explicit Sim31TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod31, 1);
            startModelKathryn();
            sim31 simulator((testSimMod31*) &d);
            simulator.simStart();
        }

    };

    Sim31TestEle ele31(31);
}