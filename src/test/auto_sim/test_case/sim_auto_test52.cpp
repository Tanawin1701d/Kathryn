//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod52: public Module{
    public:
        SlotMeta meta{{"valid", "src_idx0", "src_idx1"},
            {1,32,32}
        };
        RegSlot rs{meta};
        WireSlot ws{{"src_idx0", "src_idx1"},
            {32,32}};
        m_wire(src0get, 32);
        m_reg(idx, 4);


        explicit test_sim_mod52(int x){}

        void flow() override{

            seq{
                rs("src_idx0") <<= 24;
                rs("src_idx1") <<= 48;
                par{
                    zif(rs("src_idx0") == 24){
                        ws = rs;
                    }
                }
                idx <<= 0;
                src0get  = rs(1, 3)[idx].v();
                idx <<= idx + 1;
                src0get = rs(1, 3)[idx].v();

            }




        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test52.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test52.vcd";


    class sim52 :public SimAutoInterface{
    public:

        test_sim_mod52* _md;

        sim52(test_sim_mod52* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
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
            test_and_print("check wire_slot assign 24 ", ull(_md->ws("src_idx0")), 24);
            test_and_print("check wire_slot assign 48 ", ull(_md->ws("src_idx1")), 48);
            con_next_cycle(2);
            con_end_cycle();

            test_and_print("check wire_slot Retrieve 24 ", ull(_md->src0get), 24);

            con_next_cycle(1);
            con_end_cycle();
            test_and_print("check wire_slot No retrieve ", ull(_md->src0get), 0);

            con_next_cycle(1);
            con_end_cycle();
            test_and_print("check wire_slot No 48 ", ull(_md->src0get), 48);

        }

    };


    class Sim52TestEle: public AutoTestEle{
    public:
        explicit Sim52TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod52, 1);
            start_model_kathryn();
            sim52 simulator((test_sim_mod52*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim52TestEle ele52(52);
}