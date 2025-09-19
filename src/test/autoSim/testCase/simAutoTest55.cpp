//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    ////// simple pipeline
    class testSimMod55: public Module{
    public:
        SlotMeta meta{{"valid", "srcIdx0", "srcIdx1"},
            {1,32,32}
        };
        SlotMeta meta2{{"srcIdx0", "srcIdx1"},
                        {32,32}
        };
        Table table{meta, 2};
        RegSlot rs{meta2};
        mReg(idx, 4);
        mReg(idx2, 4);
        mReg(colIdx, 4);
        mReg(colVal, 32);


        explicit testSimMod55(int x){}

        void flow() override{
            ///// dynamic assign and slot assign test
            seq{
                par{
                    table(0)(1) <<= 1;
                    table(0)(2) <<= 2;
                    table(1)(1) <<= 3;
                    table(1)(2) <<= 4;
                    idx <<= 1;
                    idx2 <<= 0;
                    colIdx <<= 1;
                }
                par{
                    rs("srcIdx0") <<= 24;
                    rs("srcIdx1") <<= 48;
                }
                ////// dynamic read
                table[idx] <<= rs;
                table[idx2][idx] <<= 0;
            }




        }
    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest55.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest55.vcd";


    class sim55 :public SimAutoInterface{
    public:

        testSimMod55* _md;

        sim55(testSimMod55* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                              200,
                                              prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                              prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                             _md(md)
        {}

        void describeCon() override{

            // ////// skip first zync State
            conNextCycle(1);
            conEndCycle();
            testAndPrint("table row 0 col 0 check ", ull(_md->table(0)(0)), 0);
            testAndPrint("table row 0 col 1 check ", ull(_md->table(0)(1)), 1);
            testAndPrint("table row 0 col 2 check ", ull(_md->table(0)(2)), 2);
            testAndPrint("table row 0 col 0 check ", ull(_md->table(1)(0)), 0);
            testAndPrint("table row 1 col 1 check ", ull(_md->table(1)(1)), 3);
            testAndPrint("table row 1 col 2 check ", ull(_md->table(1)(2)), 4);

            conNextCycle(2); //// we skip one for system test
            conEndCycle();

            std::cout << TC_BLUE << "checking assign partial row " << TC_DEF << std::endl;
            testAndPrint("table row 0 col 0 check ", ull(_md->table(0)(0)), 0);
            testAndPrint("table row 0 col 1 check ", ull(_md->table(0)(1)), 1);
            testAndPrint("table row 0 col 2 check ", ull(_md->table(0)(2)), 2);
            testAndPrint("table row 0 col 0 check ", ull(_md->table(1)(0)), 0);
            testAndPrint("table row 1 col 1 check assigned to be 24 ", ull(_md->table(1)(1)), 24);
            testAndPrint("table row 1 col 2 check assigned to be 48 ", ull(_md->table(1)(2)), 48);

            conNextCycle(1);
            conEndCycle();

            std::cout << TC_BLUE << "checking assign each cell " << TC_DEF << std::endl;
            testAndPrint("table row 0 col 0 check ", ull(_md->table(0)(0)), 0 );
            testAndPrint("table row 0 col 1 check ", ull(_md->table(0)(1)), 1 );
            testAndPrint("table row 0 col 2 check ", ull(_md->table(0)(2)), 2 );
            testAndPrint("table row 0 col 0 check ", ull(_md->table(1)(0)), 0 );
            testAndPrint("table row 1 col 1 check assigend to be zero ", ull(_md->table(1)(1)), 0 );
            testAndPrint("table row 1 col 2 check ", ull(_md->table(1)(2)), 48);

            //
            // std::cout << TC_BLUE << "checking retrieve row " << TC_DEF << std::endl;
            // conNextCycle(1);
            // conEndCycle();
            // testAndPrint("table row 0 col 0 check ", ull(_md->table(0)(1)), 1);
            // testAndPrint("table row 0 col 1 check ", ull(_md->table(0)(2)), 2);
            //
            // std::cout << TC_BLUE << "checking retrieve row and column " << TC_DEF << std::endl;
            // conNextCycle(1);
            // conEndCycle();
            // testAndPrint("colVal ", ull(_md->colVal), 4);

        }

    };


    class Sim55TestEle: public AutoTestEle{
    public:
        explicit Sim55TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod55, 1);
            startModelKathryn();
            sim55 simulator((testSimMod55*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim55TestEle ele55(-1);
}