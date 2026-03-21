//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod55: public Module{
    public:
        SlotMeta meta{{"valid", "src_idx0", "src_idx1"},
            {1,32,32}
        };
        SlotMeta meta2{{"src_idx0", "src_idx1"},
                        {32,32}
        };
        Table table{meta, 2};
        RegSlot rs{meta2};
        m_reg(idx, 4);
        m_reg(idx2, 4);
        m_reg(col_idx, 4);
        m_reg(col_val, 32);


        explicit test_sim_mod55(int x){}

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
                    col_idx <<= 1;
                }
                par{
                    rs("src_idx0") <<= 24;
                    rs("src_idx1") <<= 48;
                }
                ////// dynamic read
                table[idx] <<= rs;
                table[idx2][idx] <<= 0;
            }




        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test55.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test55.vcd";


    class sim55 :public SimAutoInterface{
    public:

        test_sim_mod55* _md;

        sim55(test_sim_mod55* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // ////// skip first zync State
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("table row 0 col 0 check ", ull(_md->table(0)(0)), 0);
            test_and_print("table row 0 col 1 check ", ull(_md->table(0)(1)), 1);
            test_and_print("table row 0 col 2 check ", ull(_md->table(0)(2)), 2);
            test_and_print("table row 0 col 0 check ", ull(_md->table(1)(0)), 0);
            test_and_print("table row 1 col 1 check ", ull(_md->table(1)(1)), 3);
            test_and_print("table row 1 col 2 check ", ull(_md->table(1)(2)), 4);

            con_next_cycle(2); //// we skip one for system test
            con_end_cycle();

            std::cout << TC_BLUE << "checking assign partial row " << TC_DEF << std::endl;
            test_and_print("table row 0 col 0 check ", ull(_md->table(0)(0)), 0);
            test_and_print("table row 0 col 1 check ", ull(_md->table(0)(1)), 1);
            test_and_print("table row 0 col 2 check ", ull(_md->table(0)(2)), 2);
            test_and_print("table row 0 col 0 check ", ull(_md->table(1)(0)), 0);
            test_and_print("table row 1 col 1 check assigned to be 24 ", ull(_md->table(1)(1)), 24);
            test_and_print("table row 1 col 2 check assigned to be 48 ", ull(_md->table(1)(2)), 48);

            con_next_cycle(1);
            con_end_cycle();

            std::cout << TC_BLUE << "checking assign each cell " << TC_DEF << std::endl;
            test_and_print("table row 0 col 0 check ", ull(_md->table(0)(0)), 0 );
            test_and_print("table row 0 col 1 check ", ull(_md->table(0)(1)), 1 );
            test_and_print("table row 0 col 2 check ", ull(_md->table(0)(2)), 2 );
            test_and_print("table row 0 col 0 check ", ull(_md->table(1)(0)), 0 );
            test_and_print("table row 1 col 1 check assigend to be zero ", ull(_md->table(1)(1)), 0 );
            test_and_print("table row 1 col 2 check ", ull(_md->table(1)(2)), 48);

            //
            // std::cout << TC_BLUE << "checking retrieve row " << TC_DEF << std::endl;
            // con_next_cycle(1);
            // con_end_cycle();
            // test_and_print("table row 0 col 0 check ", ull(_md->table(0)(1)), 1);
            // test_and_print("table row 0 col 1 check ", ull(_md->table(0)(2)), 2);
            //
            // std::cout << TC_BLUE << "checking retrieve row and column " << TC_DEF << std::endl;
            // con_next_cycle(1);
            // con_end_cycle();
            // test_and_print("col_val ", ull(_md->col_val), 4);

        }

    };


    class Sim55TestEle: public AutoTestEle{
    public:
        explicit Sim55TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod55, 1);
            start_model_kathryn();
            sim55 simulator((test_sim_mod55*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim55TestEle ele55(55);
}