//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_SIMTEST9_H
#define KATHRYN_SIMTEST9_H


#include "kathryn.h"
#include "test/test.h"


namespace kathryn{

    class testSimMod: public Module{
    public:
        bool testAutoSkip = false;

        makeVal(bnk, 32, 48);
        makeVal(akb, 32, 48);
        makeVal(endConst, 112, 0b1111111111111111);
        makeReg(cnt, 32);
        makeReg(frd, 32);
        makeReg(end, 3);

        makeVal(one, 32,  1);


        explicit testSimMod(bool testAutoSkip): Module(){}

        void flow() override{

            seq {
                par {
                    swhile(cnt < bnk) {
                        cnt <<= cnt + one;
                    }
                    frd <<= akb;

                }
                end <<= endConst;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest9.vcd";

    class sim1 :public SimInterface{
    public:
        testSimMod* _md = nullptr;
        sim1(testSimMod* md):SimInterface(300, vcdPath),
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

#endif //KATHRYN_SIMTEST9_H

