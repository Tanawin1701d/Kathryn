//
// Created by tanawin on 22/1/2567.
//

#ifndef KATHRYN_SM_SIMTEST3_H
#define KATHRYN_SM_SIMTEST3_H


#include "kathryn.h"
#include "test/testLegacy/test.h"



namespace kathryn{

    class testSimMod: public Module{
    public:
        makeReg(a, 8);
        makeReg(b, 8);
        makeReg(c, 8);
        makeVal(iv, 8, 48);
        makeVal(iv2, 8, 0);
        makeVal(iv3, 8, 255);
        makeReg(logiAnd, 1);
        makeReg(logiOr,  1);
        makeReg(logiNot, 1);
        makeReg(logiAndExceed, 1);

        explicit testSimMod(int x): Module(){}

        void flow() override{

            seq{
                a <<= iv;
                b <<= iv2;
                logiAnd <<= iv && iv2;
                logiOr  <<= iv || iv2;
                logiNot <<= !iv2;
                logiNot <<= !iv3;
                a <<= iv3 + logiOr;
                logiAndExceed <<=  a && iv;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest3.vcd";

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
                    _md->iv.sv() = NumConverter::createValRep(8, 7);
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

#endif