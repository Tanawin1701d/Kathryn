//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod54: public Module{
    public:
        SlotMeta meta{{"valid", "src_idx0", "src_idx1"},
            {1,32,32}
        };
        Table table{meta, 2};
        RegSlot my_row{meta};
        m_reg(idx, 4);
        m_reg(col_idx, 4);
        m_reg(col_val, 32);


        explicit test_sim_mod54(int x){}

        void flow() override{
            ///// dynamic assign and slot assign test
            seq{
                par{
                    table(0)(1) <<= 1;
                    table(0)(2) <<= 2;
                    table(1)(1) <<= 3;
                    table(1)(2) <<= 4;
                    idx <<= 1;
                    col_idx <<= 1;
                }
                ////// dynamic read
                my_row  <<= table[idx].v();
                col_val <<= table.slice_by_col(1,3)[idx][col_idx].v();





            }




        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test54.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test54.vcd";


    class sim54 :public SimAutoInterface{
    public:

        test_sim_mod54* _md;

        sim54(test_sim_mod54* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // // ////// skip first zync State
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("table row 0 col 0 check ", ull(_md->table(0)(1)), 1);
            test_and_print("table row 0 col 1 check ", ull(_md->table(0)(2)), 2);
            test_and_print("table row 1 col 0 check ", ull(_md->table(1)(1)), 3);
            test_and_print("table row 1 col 1 check ", ull(_md->table(1)(2)), 4);

            std::cout << TC_BLUE << "checking retrieve row " << TC_DEF << std::endl;
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("table row 0 col 0 check ", ull(_md->table(0)(1)), 1);
            test_and_print("table row 0 col 1 check ", ull(_md->table(0)(2)), 2);

            std::cout << TC_BLUE << "checking retrieve row and column " << TC_DEF << std::endl;
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("col_val ", ull(_md->col_val), 4);

        }

    };


    class Sim54TestEle: public AutoTestEle{
    public:
        explicit Sim54TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod54, 1);
            start_model_kathryn();
            sim54 simulator((test_sim_mod54*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim54TestEle ele54(54);
}