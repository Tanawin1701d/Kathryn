//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "lib/hw/slot/table.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "carolyne/arch/caro/caro_repo.h"

namespace kathryn{

    class testSimMod44: public Module{

        mReg(i, 10);
        mReg(j, 10);
        mReg(result, 1);
    public:
        explicit testSimMod44(int x){}

        void flow() override{

            seq{
                par{i = 0; j = 0; result = 0;}
                par{
                    cwhile(i < 4){ markJoinMaster
                        i = i + 1;
                    }
                    cwhile(j < 10){j = j + 1;}
                };
                result = 1;
            }
        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest44.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest44.vcd";


    class sim44 :public SimAutoInterface{
    public:

        testSimMod44* _md;

        sim44(testSimMod44* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{}

    };


    class Sim44TestEle: public AutoTestEle{
    public:
        explicit Sim44TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod44, 1);
            startModelKathryn();
            sim44 simulator((testSimMod44*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim44TestEle ele44(44);
}