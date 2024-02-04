//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_SIMTEST10_H
#define KATHRYN_SIMTEST10_H


#include "kathryn.h"
#include "test/testLegacy/test.h"


namespace kathryn{

    class testSimMod: public Module{
    public:
        bool testAutoSkip = false;

        makeReg(a, 8);
        makeReg(b, 8);
        makeReg(result, 8);
        makeVal(resultCNA,  8, 255);
        makeVal(resultCNB,  8, 128);
        makeVal(resultCNB2, 8, 129);
        makeVal(resultCNC, 8, 20);

        makeReg(innerA, 8);
        makeReg(innerB, 8);

        makeVal(innerValA, 8, 2);
        makeVal(innerValB, 8, 3);



        explicit testSimMod(bool testAutoSkip): Module(){}

        void flow() override{

            seq {
                makeVal(bnk, 8, 48);
                makeVal(akb, 8, 49);
                innerA <<= innerValA;
                innerB <<= innerValB;
                a <<= bnk;
                b <<= akb;

                sif(a > b){
                    result <<= resultCNA;
                }selif(a < b){
                    cif(innerA > innerB){
                        result <<= resultCNB;
                    }celse{
                        result <<= resultCNB2;
                    }
                }selse{
                    result <<= resultCNC;
                }

            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest10.vcd";

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

