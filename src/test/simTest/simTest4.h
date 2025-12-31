//
// Created by tanawin on 22/1/2567.
//

#ifndef KATHRYN_SM_SIMTEST4_H
#define KATHRYN_SM_SIMTEST4_H

#include "kathryn.h"
#include "test/testLegacy/test.h"



namespace kathryn{

    class testSimMod: public Module{
    public:
        makeReg(a0, 8);
        makeReg(b0, 8);
        /** lane1*/
        makeReg(a1, 8);
        makeReg(b1, 8);
        makeReg(c1, 8);
        makeReg(d1, 8);
        /** lane2*/
        makeReg(a2, 8);
        makeReg(b2, 8);
        makeReg(c2, 8);

        makeVal(iv,   8,48);
        makeVal(iv2,  8,64);
        makeVal(zero, 8, 0);


        explicit testSimMod(int x): Module(){}

        void flow() override{
            seq {
                a0 <<= iv;
                par {
                    seq {
                        a1 <<= iv;
                        b1 <<= a1;
                        c1 <<= b1;
                        d1 <<= c1;
                    }
                    seq {
                        a2 <<= iv2;
                        b2 <<= a2;
                        c2 <<= b2;
                    }
                }
                b0 <<= iv;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest4.vcd";

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
                    //_md->iv.sv() = NumConverter::cvtStrToValRep(8, 7);
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