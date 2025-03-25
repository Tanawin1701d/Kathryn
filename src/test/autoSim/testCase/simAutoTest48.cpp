//
// Created by tanawin on 23/3/2025.
//


#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod48: public Module{
    public:
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        mReg(e, 8);
        mReg(f, 8);


        explicit testSimMod48(int x): Module(){}

        void flow() override{

            seq{
                par{
                    a = 5; b = 0;
                    c = 0; d = 0;
                }
                par{
                    ///// fetch pipe
                    pip("fetch"){ autoStart
                        pipTran("decode"){
                            b <<= a;
                            a <<= a + 1;
                        }
                    }
                    ///// decode pipe
                    pip("decode"){
                        pipMTran{
                            zif(b(0) == 0){ c <<= b; }
                            zif(b(0) == 1){ d <<= b; }
                            tranToWC("exec0", b(0) == 0);
                            tranToWC("exec1", b(0) == 1);

                        }
                    }

                    pip("exec0"){e <<= c;}

                    pip("exec1"){f <<= d;}

                }

            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest48.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest48.vcd";


    class sim48 :public SimAutoInterface{
    public:

        testSimMod48* _md;

        sim48(testSimMod48* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                                _md(md)
        {}

        void describeCon() override{
            std::cout << TC_BLUE << "test pipe multipath" << TC_DEF << std::endl;
            conNextCycle(2);
            testAndPrint("testPipVal: B", ull(_md->b), 5);
            testAndPrint("testPipVal: B", ull(_md->c), 0);
            testAndPrint("testPipVal: B", ull(_md->d), 0);
            conNextCycle(1);
            testAndPrint("testPipVal: B", ull(_md->b), 6);
            testAndPrint("testPipVal: B", ull(_md->c), 0);
            testAndPrint("testPipVal: B", ull(_md->d), 5);
            conNextCycle(1);
            testAndPrint("testPipVal: B", ull(_md->b), 7);
            testAndPrint("testPipVal: B", ull(_md->c), 6);
            testAndPrint("testPipVal: B", ull(_md->d), 5);
            conNextCycle(1);
            testAndPrint("testPipVal: B", ull(_md->b), 8);
            testAndPrint("testPipVal: B", ull(_md->c), 6);
            testAndPrint("testPipVal: B", ull(_md->d), 7);
            conNextCycle(1);
            testAndPrint("testPipVal: B", ull(_md->b), 9);
            testAndPrint("testPipVal: B", ull(_md->c), 8);
            testAndPrint("testPipVal: B", ull(_md->d), 7);
            conNextCycle(1);
            testAndPrint("testPipVal: B", ull(_md->b), 10);
            testAndPrint("testPipVal: B", ull(_md->c), 8);
            testAndPrint("testPipVal: B", ull(_md->d), 9);

        }



    };


    class Sim48TestEle: public AutoTestEle{
    public:
        explicit Sim48TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod48, 1);
            startModelKathryn();
            sim48 simulator((testSimMod48*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim48TestEle ele48(48);
}