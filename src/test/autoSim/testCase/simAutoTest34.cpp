//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod34: public Module{
    public:

        std::vector<Reg*> cnts;

        explicit testSimMod34(int x): Module(){}

        void flow() override{

            for (int i = 0; i < 200; i++){
                mReg(a, 8);
                cnts.push_back(&a);
                *cnts[i] <<= *cnts[i] + 1;
            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest34.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest34.vcd";


    class sim34 :public SimAutoInterface{
    public:

        testSimMod34* _md;

        sim34(testSimMod34* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                                 10000000,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                                _md(md)
        {}

        void describeCon() override{

        }
    };


    class Sim34TestEle: public AutoTestEle{
    public:
        explicit Sim34TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod34, 1);
            startModelKathryn();
            sim34 simulator((testSimMod34*) &d, _simId, prefix);
            auto start = std::chrono::steady_clock::now();
            simulator.simStart();
            std::cout << TC_GREEN << "--------------------------------" << std::endl;
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
        }

    };

    Sim34TestEle ele34(-1);
}