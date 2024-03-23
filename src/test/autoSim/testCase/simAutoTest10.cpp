//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod10: public Module{
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



        explicit testSimMod10(bool testAutoSkip): Module(){}

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

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest10.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest10.vcd";


    class sim10 :public SimAutoInterface{
    public:

        testSimMod10* _md;

        sim10(testSimMod10* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(8);

            sim {
                ValRep testVal = NumConverter::createValRep(8, 129);
                testAndPrint("check cifelse in sifelse", _md->result.sv(), testVal);
            };

        }

        void simDriven() override{
            incCycle(2);

            sim {
                //_md->iv.sv() = NumConverter::cvtStrToValRep(8, 48);
            };
            incCycle(1);
        }

    };


    class Sim10TestEle: public AutoTestEle{
    public:
        explicit Sim10TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod10, 1);
            startModelKathryn();
            sim10 simulator((testSimMod10*) &d);
            simulator.simStart();
        }

    };

    Sim10TestEle ele10(10);

    ///sim2 testCase2;

}