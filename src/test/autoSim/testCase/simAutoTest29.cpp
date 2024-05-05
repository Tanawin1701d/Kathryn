//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod29: public Module{
    public:

        mReg(a, 8);
        mReg(a2, 8);
        mVal(b, 8, 48);
        mReg(c, 1);
        mReg(d, 1);
        mWire(is, 1);

        explicit testSimMod29(int x): Module(){}

        void flow() override{

            cwhile(  c == 1 ){

                cwhile(true){
                    intrStart(is);
                    seq{
                        cif(a == 48){
                            sbreak;
                        }celse{
                            a = a + 1;
                        }
                    }
                }

            }

            seq{
                syWait(5);
                par {
                    is = 1;
                    c  = 1;
                }
            }


        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest29.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest29.vcd";


    class sim29 :public SimAutoInterface{
    public:

        testSimMod29* _md;

        sim29(testSimMod29* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void describeCon() override{

            conNextCycle(6);
            for(int i =1; i <= 48; i++){
                if (i % 10 == 1)
                    testAndPrint("check intr par a start", ull(_md->a),i);
                conNextCycle(1);
            }

        }

    };


    class Sim29TestEle: public AutoTestEle{
    public:
        explicit Sim29TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod29, 1);
            startModelKathryn();
            sim29 simulator((testSimMod29*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim29TestEle ele29(29);
}