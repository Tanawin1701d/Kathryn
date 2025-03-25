//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod23: public Module{
    public:
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        mReg(e, 8);
        mReg(f, 8);

        explicit testSimMod23(int x): Module(){}

        void flow() override{

            seq{
                par{
                    a = 0; b = 0;
                    c = 0; d = 0;
                }
                par{
                    ///// fetch pipe
                    pip("fetch"){ autoStart
                        pipTran("decode"){
                            ////arbReq("specTagGen");
                            a <<= a + 1;
                        }
                    }
                    ///// decode pipe
                    pip("decode"){
                        seq{
                            b <<= a;
                            cif((a + 1) == 3){
                                syWait(3);
                            }
                        }
                    }
                }

            }
        }


        /**
         *      --------- req ----->
         *      <-------- offer ----
         *      --------- occ (when all related signal is done) ----->
         * **/

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest23.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest23.vcd";


    class sim23 :public SimAutoInterface{
    public:

        testSimMod23* _md;

        sim23(testSimMod23* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                                _md(md)
        {}

        void describeCon() override{

            conNextCycle(2);
            testAndPrint("testPipVal: A", ull(_md->a), 1);
            testAndPrint("testPipVal: B", ull(_md->b), 0);
            conNextCycle(1);
            testAndPrint("testPipVal: A", ull(_md->a), 2);
            testAndPrint("testPipVal: B", ull(_md->b), 1);
            conNextCycle(4);
            testAndPrint("testPipVal: A", ull(_md->a), 3);
            testAndPrint("testPipVal: B", ull(_md->b), 2);
            conNextCycle(1);
            testAndPrint("testPipVal: A", ull(_md->a), 4);
            testAndPrint("testPipVal: B", ull(_md->b), 3);

        }



    };


    class Sim23TestEle: public AutoTestEle{
    public:
        explicit Sim23TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod23, 1);
            startModelKathryn();
            sim23 simulator((testSimMod23*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim23TestEle ele23(23);
}