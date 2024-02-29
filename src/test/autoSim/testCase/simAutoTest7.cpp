//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod7: public Module{
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


        explicit testSimMod7(bool testAutoSkip): Module(){}

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

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest7.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest7.vcd";


    class sim7 :public SimAutoInterface{
    public:

        testSimMod7* _md;

        sim7(testSimMod7* md):SimAutoInterface(2,
                                              100,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void simAssert() override{


            incCycle(6);

            sim {
                ValRep testVal = NumConverter::createValRep(32, 11);
                testAndPrint("check middle con", _md->s.sv(), testVal);
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


    class Sim7TestEle: public AutoTestEle{
    public:
        explicit Sim7TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod7, 1);
            sim7 simulator((testSimMod7*) &d);
            simulator.simStart();
        }

    };

    Sim7TestEle ele7(7);

    ///sim2 testCase2;




}