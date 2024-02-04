//
// Created by tanawin on 22/1/2567.
//

#ifndef KATHRYN_SM_SIMTEST1_H
#define KATHRYN_SM_SIMTEST1_H

#include "kathryn.h"
#include "test/testLegacy/test.h"



namespace kathryn{

    class testSimMod: public Module{
    public:
        makeReg(a, 8);
        makeReg(b, 8);
        makeReg(c, 8);
        makeVal(iv, 8, 0b10101010);

        explicit testSimMod(int x): Module(){}

        void flow() override{

            makeVal(bnk, 8, 48);
            makeVal(one, 8, 1);
            makeVal(two, 8, 2);
            makeVal(maxer, 8, 255);
            makeReg(ota, 8);
            makeReg(maxReg, 8);
            makeReg(bwOrReg, 8);


            par{
                a <<= iv;
                b <<= iv;
                c <<= bnk;
                ota <<= bnk - one;
                maxReg <<= maxer + two;
                bwOrReg <<= bnk | one;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest2.vcd";

    class sim1 :public SimInterface{
    public:
        testSimMod* _md = nullptr;
        sim1(testSimMod* md):SimInterface(100, vcdPath),
                             _md(md){
            assert(_md != nullptr);
        }

        void describe() override{

            incCycle(2);

            for (int i = 0; i < 100; i++) {
                sim {
                    _md->iv.sv() = NumConverter::cvtStrToValRep(8, 7);
                };
                incCycle(1);
            }


        }
    };

    class test1: public Test{
    public:

        void test() override {
            makeMod(tm, testSimMod, 0);

            /**logger */
            auto mdLogVal = new MdLogVal();
            tm.addMdLog(mdLogVal);
            logMD("tm", mdLogVal);
            //////////////////////////////////
            sim1 s((testSimMod*)(&tm));
            s.simStart();

        }
    };



}

#endif //KATHRYN_SIMTEST1_H
