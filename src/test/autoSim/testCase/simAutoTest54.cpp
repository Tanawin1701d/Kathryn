//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod54: public Module{
    public:
        SlotMeta meta{{"valid", "srcIdx0", "srcIdx1"},
            {1,32,32}
        };
        Table table{meta, 2};
        RegSlot myRow{meta};
        mReg(idx, 4);
        mReg(colIdx, 4);
        mReg(colVal, 32);


        explicit testSimMod54(int x){}

        void flow() override{
            ///// dynamic assign and slot assign test
            seq{
                par{
                    table(0)(1) <<= 1;
                    table(0)(2) <<= 2;
                    table(1)(1) <<= 3;
                    table(1)(2) <<= 4;
                    idx <<= 1;
                    colIdx <<= 1;
                }
                ////// dynamic read
                myRow  <<= table[idx].v();
                colVal <<= table.sliceByCol(1,3)[idx][colIdx].v();





            }




        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest54.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest54.vcd";


    class sim54 :public SimAutoInterface{
    public:

        testSimMod54* _md;

        sim54(testSimMod54* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // // ////// skip first zync State
            conNextCycle(1);
            conEndCycle();
            testAndPrint("table row 0 col 0 check ", ull(_md->table(0)(1)), 1);
            testAndPrint("table row 0 col 1 check ", ull(_md->table(0)(2)), 2);
            testAndPrint("table row 1 col 0 check ", ull(_md->table(1)(1)), 3);
            testAndPrint("table row 1 col 1 check ", ull(_md->table(1)(2)), 4);

            std::cout << TC_BLUE << "checking retrieve row " << TC_DEF << std::endl;
            conNextCycle(1);
            conEndCycle();
            testAndPrint("table row 0 col 0 check ", ull(_md->table(0)(1)), 1);
            testAndPrint("table row 0 col 1 check ", ull(_md->table(0)(2)), 2);

            std::cout << TC_BLUE << "checking retrieve row and column " << TC_DEF << std::endl;
            conNextCycle(1);
            conEndCycle();
            testAndPrint("colVal ", ull(_md->colVal), 4);

        }

    };


    class Sim54TestEle: public AutoTestEle{
    public:
        explicit Sim54TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod54, 1);
            startModelKathryn();
            sim54 simulator((testSimMod54*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim54TestEle ele54(54);
}