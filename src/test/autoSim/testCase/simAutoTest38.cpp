//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod38_subMod: public Module{
    public:
        mInput(sa, 8);
        mOutput(sb, 8);
        mWire(ss, 8);

        explicit testSimMod38_subMod(Operable& ipt): Module(){
            sa = ipt;
        }

        void flow() override{
            ss = sa + 1;
            sb = ss;
        }

    };

    class testSimMod38: public Module{
    public:
        mReg(a, 8);

        explicit testSimMod38(int x): Module(){

            mMod(sub, testSimMod38_subMod, a);

        }

        void flow() override{
            seq{
                a <<= 0;
                cwhile(true){
                    a <<= a + 1;
                }
            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest38.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest38.vcd";


    class sim38 :public SimAutoInterface{
    public:

        testSimMod38* _md;

        sim38(testSimMod38* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                                _md(md)
        {}

        void describeCon() override{

        }
    };


    class Sim38TestEle: public AutoTestEle{
    public:
        explicit Sim38TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod38, 1);
            startModelKathryn();
            sim38 simulator((testSimMod38*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim38TestEle ele38(-1);
}