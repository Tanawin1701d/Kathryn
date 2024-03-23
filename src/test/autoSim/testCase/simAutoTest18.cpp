//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod18: public Module{
    public:

        makeReg(i, 6);
        makeReg(k, 8);
        makeReg(b, 6);
        makeReg(c, 32);
        makeMem(storage, 48, 8);


        explicit testSimMod18(int x): Module(){}

        void flow() override{

            seq{
                i <<= 0;
                cwhile(i < 47){
                    /////// max in this scope is 47
                    par {
                        storage[i] <<= k;
                        i <<= i + 1;
                        k <<= k + 1;
                    }
                }
                i <<= 0;
                i <<= 0;
                cwhile(i < 47){
                    par {
                        i <<= i + 1;
                        b <<= storage[i];
                    }
                }
            }

        }
    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest18.vcd";
    static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest18.vcd";


    class sim18 :public SimAutoInterface{
    public:

        testSimMod18* _md;

        sim18(testSimMod18* md):SimAutoInterface(2,
                                              200,
                                              vcdPath,
                                              profilePath),
                             _md(md)
        {}

        void describeCon() override{
            conNextCycle(52);
            for (int i =0; i < 48; i++){
                testAndPrint("checkMem with new system : " + std::to_string(i), ull(_md->storage.v(i)), i);
            }
            for(int i = 0; i < 48; i++){
                ValRep testVal = NumConverter::createValRep(6, i);

                testAndPrint("checkMem : " + std::to_string(i), _md->b.sv(), testVal);
                conNextCycle(1);
            }
        }

    };


    class Sim18TestEle: public AutoTestEle{
    public:
        explicit Sim18TestEle(int id): AutoTestEle(id){}
        void start() override{
            makeMod(d, testSimMod18, 1);
            startModelKathryn();
            sim18 simulator((testSimMod18*) &d);
            simulator.simStart();
        }

    };

    Sim18TestEle ele18(18);
}