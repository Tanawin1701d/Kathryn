//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod10: public Module{
    public:
        bool testAutoSkip = false;

        mReg(a, 8);
        mReg(b, 8);
        mReg(result, 8);
        mVal(resultCNA,  8, 255);
        mVal(resultCNB,  8, 128);
        mVal(resultCNB2, 8, 129);
        mVal(resultCNC, 8, 20);

        mReg(innerA, 8);
        mReg(innerB, 8);

        mVal(innerValA, 8, 2);
        mVal(innerValB, 8, 3);



        explicit testSimMod10(bool testAutoSkip): Module(){}

        void flow() override{

            seq {
                mVal(bnk, 8, 48);
                mVal(akb, 8, 49);
                innerA <<= innerValA;
                innerB <<= innerValB;
                a <<= bnk;
                b <<= akb;

                sif(a > b){
                    result <<= resultCNA;
                }
                selif(a < b){
                    cif(innerA > innerB){
                        result <<= resultCNB;
                    }celse{
                        result <<= resultCNB2;
                    }
                }
                selse{
                    result <<= resultCNC;
                }

            }

        }

    };

    class sim10 :public SimAutoInterface{
    public:

        testSimMod10* _md;

        sim10(testSimMod10* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(8);

            sim {
                ull testVal = 129;
                testAndPrint("check cifelse in sifelse", (ull)_md->result, testVal);
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


    class Sim10TestEle: public AutoTestEle{
    public:
        explicit Sim10TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod10, 1);
            startModelKathryn();
            sim10 simulator((testSimMod10*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim10TestEle ele10(-1);

    ///sim2 testCase2;

}