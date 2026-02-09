//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod62: public Module{
    public:

        mReg(switchVal, 3);
        mReg(subCheck, 1);
        mReg(a, 32);
        mReg(b, 32);

        explicit testSimMod62(int x){}

        void flow() override{
            ///// test priority of the zif block

            seq{
                par{
                    switchVal <<= 4;
                    subCheck  <<= 1;
                }

                par{
                    ztate(switchVal){
                        zcase(0b100){
                            a <<=  9;
                            b <<= 24;
                            zif(subCheck){
                                b <<= 48;
                            }
                        }
                        zcase(0b001){
                            a <<= 10;
                            b <<= 107;
                            b <<= 108;
                        }
                        zcasedef{
                            b <<= 404;
                        }
                    }
                }
            }


        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest62.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest62.vcd";


    class sim62 :public SimAutoInterface{
    public:

        testSimMod62* _md;

        sim62(testSimMod62* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            conNextCycle(1);
            testAndPrint("switchVal before active", ull(_md->switchVal),   4 );
            testAndPrint("subCheck before active",  ull(_md->subCheck),    1 );
            testAndPrint("a before active",         ull(_md->a),           0 );
            testAndPrint("b before active",         ull(_md->b),           0 );
            conNextCycle(1);
            testAndPrint("a after active",         ull(_md->a),  9);
            testAndPrint("b after active",         ull(_md->b), 48);
        }

    };


    class Sim62TestEle: public AutoTestEle{
    public:
        explicit Sim62TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod62, 1);
            startModelKathryn();
            sim62 simulator((testSimMod62*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim62TestEle ele62(62);
}