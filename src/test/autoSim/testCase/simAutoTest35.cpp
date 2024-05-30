//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod35: public Module{
    public:
        mReg(a, 8);
        mReg(b, 8);


        explicit testSimMod35(int x): Module(){}

        void flow() override{

            seq{
                cwhile(a < 16){
                    a <<= a + 1;
                    for (int i = 0; i < 2; i++){
                        b <<= a + 2;
                    }
                }
            }
        }

    };

    class sim35 :public SimAutoInterface{
    public:

        testSimMod35* _md = nullptr;

        sim35(testSimMod35* md, int idx, const std::string& prefix):SimAutoInterface(idx,100,
                                                                                  prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                                                  prefix + "simAutoResult"+std::to_string(idx)+".prof"),
        _md(md)
        {}
        void simAssert() override{}
        void simDriven() override{}
    };

    class Sim35TestEle: public AutoTestEle{
    public:
        explicit Sim35TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod35, 1);
            startModelKathryn();
            sim35 simulator((testSimMod35*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim35TestEle ele35(35);

}