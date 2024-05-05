//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod: public Module{
    public:
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mVal(iv, 8, 0b10101010);

        explicit testSimMod(int x): Module(){}

        void flow() override{

            seq{
                a <<= iv;
                b <<= a;
                c <<= b;
                mVal(zero, 8, 0);
                a <<= zero;
                b <<= zero;
                c <<= zero;
                for (int i = 0; i < 4; i++){
                    mVal(x, 8, i+1);
                    a <<= x;
                }
                mVal(y, 8, 3);
                a <<= a << y;
            }

//
//            seq{
//                cif(a < b){
//                    a <<= b;
//                }celse{
//                    b <<= a;
//                }
//            }
//
//            cwhile(a<b){
//                a <<= a + 1;
//            }

        }

    };

    class sim1 :public SimAutoInterface{
    public:

        testSimMod* _md = nullptr;

        sim1(testSimMod* md, int idx, const std::string& prefix):SimAutoInterface(idx,100,
                                                                                  prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                                                  prefix + "simAutoResult"+std::to_string(idx)+".prof"),
        _md(md)
        {}

        void simAssert() override{
            incCycle(13);
            sim {
                ValRep testVal = NumConverter::createValRep(8, 32);
                testAndPrint("check End Val", _md->a.sv(), testVal);
            };
            sim{
                ValRep testVal = NumConverter::createValRep(8, 32);
                ull cval(_md->a);
                ValRep cValTest = NumConverter::createValRep(8, cval);
                testAndPrint("check C Val", cValTest, testVal);
            };
        }

        void simDriven() override{
            incCycle(2);
            for (int i = 0; i < 100; i++) {
                sim {
                    _md->iv.sv() = NumConverter::createValRep(8, 7);
                    _md->c = 7;
                };
                incCycle(1);
            }
        }
    };

    class Sim1TestEle: public AutoTestEle{
    public:
        explicit Sim1TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod, 1);
            startModelKathryn();
            sim1 simulator((testSimMod*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim1TestEle ele1(1);

}