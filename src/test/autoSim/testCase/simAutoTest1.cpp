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

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest1.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest1.vcd";

    class sim1 :public SimAutoInterface{
    public:

        testSimMod* _md = nullptr;

        sim1(testSimMod* md):SimAutoInterface(0,100,vcdPath, profilePath),
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
        void start() override{
            mMod(d, testSimMod, 1);
            startModelKathryn();
            sim1 simulator((testSimMod*) &d);
            simulator.simStart();
        }

    };

    Sim1TestEle ele1(1);

}