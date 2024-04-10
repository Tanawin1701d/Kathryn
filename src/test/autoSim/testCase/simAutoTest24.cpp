//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod24: public Module{
    public:

        makeReg (i, 6);
        makeReg (k, 8);
        makeWire(b, 6);
        makeReg(c, 32);
        makeMem(storage, 48, 8);


        explicit testSimMod24(int x): Module(){}

        void flow() override{

            seq{
                i = 0;
                cwhile(i < 48){
                    par {
                        storage[i] = k;
                        i          = i + 1;
                        k          = k + 1;
                    }
                }
                i = 0;
                cwhile(i < 48){
                    par {
                        i = i + 1;
                        b = storage[i];
                    }
                }
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest24.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest24.vcd";


    class sim24 :public SimAutoInterface{
    public:

        testSimMod24* _md;

        sim24(testSimMod24* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void describeCon() override{
//            conNextCycle(52);
//            for (int i =0; i < 48; i++){
//                testAndPrint("checkMem with new system : " + std::to_string(i), ull(_md->storage.v(i)), i);
//            }
//            for(int i = 0; i < 48; i++){
//                ValRep testVal = NumConverter::createValRep(6, i);
//
//                testAndPrint("checkMem : " + std::to_string(i), _md->b.sv(), testVal);
//                conNextCycle(1);
//            }
        }

    };


    class Sim24TestEle: public AutoTestEle{
    public:
        explicit Sim24TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod24, 1);
            startModelKathryn();
            sim24 simulator((testSimMod24*) &d);
            simulator.simStart();
        }

    };

    Sim24TestEle ele24(24);
}