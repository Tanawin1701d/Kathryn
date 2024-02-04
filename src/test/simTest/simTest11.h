//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_SIMTEST11_H
#define KATHRYN_SIMTEST11_H


#include "kathryn.h"
#include "test/test.h"


namespace kathryn{

    class testSimMod: public Module{
    public:
        bool testAutoSkip = false;

        makeReg(a, 8);
        makeReg(b, 8);
        makeVal(cond, 3, 2);
        makeVal(one, 8, 1);
        makeVal(two, 8, 2);
        makeVal(three, 8, 3);

        explicit testSimMod(bool testAutoSkip): Module(){}

        void flow() override{

            cwhile(cond){
                zif(a > b){
                    a <<= a + one;
                }zelif(a < b){
                    a <<= a + two;
                    zif(a > b){
                        b <<= b - one;
                    }zelse{
                        b <<= b - two;
                    }
                }
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest11.vcd";

    class sim1 :public SimInterface{
    public:
        testSimMod* _md = nullptr;
        sim1(testSimMod* md):SimInterface(300, vcdPath),
                             _md(md){
            assert(_md != nullptr);
        }

        void describe() override{

            incCycle(2);

            for (int i = 0; i < 1; i++) {
                sim {
                    _md->b.sv() = NumConverter::cvtStrToValRep(8, 48);
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

#endif //KATHRYN_SIMTEST11_H

