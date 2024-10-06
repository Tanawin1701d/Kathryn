//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod39: public Module{
    public:

        mReg(a  , 138);
        mReg(b  , 138);

        explicit testSimMod39(int x): Module(){}

        void flow() override{

            seq{
                a <<= b;
            }

        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest39.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest39.vcd";


    class sim39 :public SimAutoInterface{
    public:

        testSimMod39* _md;

        sim39(testSimMod39* md, int idx, const std::string& prefix):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof"),
                             _md(md)
        {}

        void describeCon() override{
            _md->b.s(hex(136, "F0000000000000001"));
            conNextCycle(1);

            testAndPrint("a[0]", _md->a.v().getLargeVal()[0], 1);
            testAndPrint("a[1]", _md->a.v().getLargeVal()[1], 15);
            testAndPrint("a[2]", _md->a.v().getLargeVal()[2], 0);

            // auto x = _md->a.v().getLargeVal();
            // std::cout << "get large value" << std::endl;
            // for (ull v: x){ std::cout << v << std::endl; }
            // std::cout << "end get large value" << std::endl;
        }



    };


    class Sim39TestEle: public AutoTestEle{
    public:
        explicit Sim39TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod39, 1);
            startModelKathryn();
            sim39 simulator((testSimMod39*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim39TestEle ele39(39);

}