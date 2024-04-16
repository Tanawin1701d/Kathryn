//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod30: public Module{
    public:

        makeReg (a, 8);
        makeWire(is, 1);
        makeReg (b, 8);


        explicit testSimMod30(int x): Module(){}

        void flow() override{

            cwhile(true){
                intrReset(is);
                seq{
                    par{
                        cwhile(a < 48){
                            cif(a(0)){
                                seq {
                                    a = a + 1;
                                    a = a + 1;
                                }
                            }celse{
                                a = a + 2;
                            }
                        }
                        b = b + 1;
                    }
                    a = a + 1;
                }
            }

            seq{
                syWait(5);
                par {
                    is = 1;
                }
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest30.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest30.vcd";


    class sim30 :public SimAutoInterface{
    public:

        testSimMod30* _md;

        sim30(testSimMod30* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void describeCon() override{

//            conNextCycle(6);
//            for(int i =1; i <= 48; i++){
//                if (i % 10 == 1)
//                    testAndPrint("check intr par a start", ull(_md->a),i);
//                conNextCycle(1);
//            }

        }

    };


    class Sim30TestEle: public AutoTestEle{
    public:
        explicit Sim30TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod30, 1);
            startModelKathryn();
            sim30 simulator((testSimMod30*) &d);
            simulator.simStart();
        }

    };

    Sim30TestEle ele30(30);
}