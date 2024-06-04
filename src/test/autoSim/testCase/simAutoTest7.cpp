//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod7: public Module{
    public:
        bool testAutoSkip = false;
        mVal(bnk, 32, 48);
        mVal(zero, 32, 0);
        mVal(hf, 20, 64);
        mVal(nine, 20, 9);
        mVal(ele, 20, 11);
        mReg(a , 32);
        mReg(b , 32);
        mReg(c , 32);

        mReg(s , 32);
        mReg(s2 , 32);
        mReg(s3 , 32);

        mReg(cnt, 2);
        mVal(one, 2, 1);
        mVal(maxCnt, 2, 0);


        explicit testSimMod7(bool testAutoSkip): Module(){}

        void flow() override{

            seq{
                a <<= bnk;
                cnt <<= zero;
                b <<= zero;
                par {
                    cif(a > hf) {
                        s <<= nine;
                        s2 <<= nine;
                        s3 <<= nine;
                    }celif(a > nine) {
                        seq {
                            s <<= ele;
                            s2 <<= ele;
                            cwhile(cnt == maxCnt){
                                /** to test sync Reg*/
                                cnt <<= cnt + one;
                            }
                            s3 <<= ele;
                        }
                    }
                }
                c <<= bnk;
            }

        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest7.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest7.vcd";


    class sim7 :public SimAutoInterface{
    public:

        testSimMod7* _md;

        sim7(testSimMod7* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              100,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(6);

            sim {
                ull testVal = 11;
                testAndPrint("check middle con", (ull)_md->s, testVal);
            };

        }

        void simDriven() override{
            incCycle(2);

            sim {
                //_md->iv.sv() = NumConverter::cvtStrToValRep(8, 48);
            };
            incCycle(1);
        }

    };


    class Sim7TestEle: public AutoTestEle{
    public:
        explicit Sim7TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod7, 1);
            startModelKathryn();
            sim7 simulator((testSimMod7*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim7TestEle ele7(7);

    ///sim2 testCase2;




}