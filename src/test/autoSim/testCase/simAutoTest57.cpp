//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod57: public Module{
    public:
        SlotMeta meta{{"valid", "srcIdx0", "srcIdx1"},
            {1,32,32}
        };

        Table table{meta, 2};
        WireSlot ws{meta, "result"};
        mWire(idx, 1);


        explicit testSimMod57(int x){}

        void flow() override{
            ///// dynamic assign and slot assign test
            seq{

                par{
                    table(0)(1) <<= 48;
                    table(0)(2) <<= 2;
                    table(1)(1) <<= 24;
                    table(1)(2) <<= 4;
                }
                auto compare = [](WireSlot& lhs, Operable* lidx, WireSlot& rhs, Operable* ridx) -> Operable&{
                    return lhs(1) < rhs(1);
                };
                par{
                    std::tie(ws, idx) = table.doReducBinIdx(compare);
                }

            }




        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest57.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest57.vcd";


    class sim57 :public SimAutoInterface{
    public:

        testSimMod57* _md;

        sim57(testSimMod57* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // // ////// skip first zync State
            conNextCycle(1);
            conEndCycle();
            testAndPrint("check result row val valid "  , ull(_md->ws(0)), 0);
            testAndPrint("check result row val srcIdx0 ", ull(_md->ws(1)), 24);
            testAndPrint("check result row val srcIdx1 ", ull(_md->ws(2)), 4);
            testAndPrint("check result idx "            , ull(_md->idx), 1);

        }

    };


    class Sim57TestEle: public AutoTestEle{
    public:
        explicit Sim57TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod57, 1);
            startModelKathryn();
            sim57 simulator((testSimMod57*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim57TestEle ele57(-2);
}