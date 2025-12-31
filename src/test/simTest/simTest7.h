//
// Created by tanawin on 30/1/2567.
//

#ifndef KATHRYN_SIMTEST7_H
#define KATHRYN_SIMTEST7_H


#include "kathryn.h"
#include "test/testLegacy/test.h"


namespace kathryn{

    class testSimMod: public Module{
    public:
        bool testAutoSkip = false;
        makeVal(bnk, 32, 48);
        makeVal(zero, 32, 0);
        makeVal(hf, 20, 64);
        makeVal(nine, 20, 9);
        makeVal(ele, 20, 11);
        makeReg(a , 32);
        makeReg(b , 32);
        makeReg(c , 32);

        makeReg(s , 32);
        makeReg(s2 , 32);
        makeReg(s3 , 32);

        makeReg(cnt, 2);
        makeVal(one, 2, 1);
        makeVal(maxCnt, 2, 0);


        explicit testSimMod(bool testAutoSkip): Module(){}

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

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest7.vcd";

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
            makeMod(tm, testSimMod, false);

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

#endif //KATHRYN_SIMTEST7_H
