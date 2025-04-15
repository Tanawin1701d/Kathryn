//
// Created by tanawin on 23/3/2025.
//


#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod49: public Module{
    public:
        mReg (a  , 8);
        mReg (b  , 8);
        mReg (c  , 8);
        mReg (d  , 8);
        mWire(x  , 1);
        mWire(st0, 1);
        mWire(st1, 1);


        explicit testSimMod49(int x): Module(){}

        void flow() override{

            cwhile(true){
                offer("test"){
                    ofc("a"){ a <<= a + 1; }
                    ofcc("b", x){ b <<=  b + 1; }
                }
            }


            seq{
                syWait(5);
                par{
                    zif(getOffer("test", "a")){
                        st0 = 1;
                        ackOffer("test", "a", st0);
                    }
                }
                syWait(5);
                par{
                    zif(getOffer("test", "b")){
                        st1 = 1;
                        ackOffer("test", "b", st1);
                    }
                }

            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest49.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest49.vcd";


    class sim49 :public SimAutoInterface{
    public:

        testSimMod49* _md;

        sim49(testSimMod49* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
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


    class Sim49TestEle: public AutoTestEle{
    public:
        explicit Sim49TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod49, 1);
            startModelKathryn();
            sim49 simulator((testSimMod49*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim49TestEle ele49(49);
}