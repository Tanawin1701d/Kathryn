//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod53: public Module{
    public:
        SlotMeta meta{{"valid", "src_idx0", "src_idx1"},
            {1,32,32}
        };
        SlotMeta meta2{{"src_idx3", "src_idx4"},
                        {32, 32}
        };
        RegSlot rs {meta };
        RegSlot rs2{meta + meta2};
        m_reg(idx, 4);


        explicit test_sim_mod53(int x){}

        void flow() override{
            ///// dynamic assign and slot assign test
            seq{
                rs("src_idx0") <<= 24;
                rs("src_idx1") <<= 48;
                idx <<= 0;
                rs(1, 3)[idx] <<= 26;
                idx <<= idx + 1;
                rs(1, 3)[idx] <<= 50;
                rs2 <<= rs;


            }




        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test53.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test53.vcd";


    class sim53 :public SimAutoInterface{
    public:

        test_sim_mod53* _md;

        sim53(test_sim_mod53* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // // ////// skip first zync State
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("check reg_slot assign 24 ", ull(_md->rs("src_idx0")), 24);
            test_and_print("check reg_slot assign 0 ", ull(_md->rs("src_idx1")), 0);
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("check reg_slot assign 24 ", ull(_md->rs("src_idx0")), 24);
            test_and_print("check reg_slot assign 48 ", ull(_md->rs("src_idx1")), 48);
            con_next_cycle(2);
            con_end_cycle();
            test_and_print("check reg_slot assign 26 ", ull(_md->rs("src_idx0")), 26);
            test_and_print("check reg_slot assign 48 ", ull(_md->rs("src_idx1")), 48);
            con_next_cycle(2);
            con_end_cycle();
            test_and_print("check reg_slot assign 26 ", ull(_md->rs("src_idx0")), 26);
            test_and_print("check reg_slot assign 50 ", ull(_md->rs("src_idx1")), 50);
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("check whole reg_slot assign 26 ", ull(_md->rs2("src_idx0")), 26);
            test_and_print("check whole reg_slot assign 50 ", ull(_md->rs2("src_idx1")), 50);
            test_and_print("check whole reg_slot other field (src_idx3)", ull(_md->rs2("src_idx3")), 0);
            test_and_print("check whole reg_slot other field (src_idx4)", ull(_md->rs2("src_idx4")), 0);

            // test_and_print("check wire_slot assign 24 ", ull(_md->ws("src_idx0")), 24);
            // test_and_print("check wire_slot assign 48 ", ull(_md->ws("src_idx1")), 48);
            // con_next_cycle(2);
            // con_end_cycle();
            //
            // test_and_print("check wire_slot Retrieve 24 ", ull(_md->src0get), 24);
            //
            // con_next_cycle(1);
            // con_end_cycle();
            // test_and_print("check wire_slot No retrieve ", ull(_md->src0get), 0);
            //
            // con_next_cycle(1);
            // con_end_cycle();
            // test_and_print("check wire_slot No 48 ", ull(_md->src0get), 48);

        }

    };


    class Sim53TestEle: public AutoTestEle{
    public:
        explicit Sim53TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod53, 1);
            start_model_kathryn();
            sim53 simulator((test_sim_mod53*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim53TestEle ele53(53);
}