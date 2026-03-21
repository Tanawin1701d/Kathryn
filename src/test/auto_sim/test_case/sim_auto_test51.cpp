//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod51: public Module{
    public:
        m_reg(a, 32);
        m_reg(b, 32);
        m_reg(c, 32);
        m_wire(purpose, 32);
        SyncMeta fetch{"fetch"};
        SyncMeta decode{"decode"};


        explicit test_sim_mod51(int x){}

        void flow() override{
            a.make_reset_event();

            pip(fetch){ auto_sync
                zync(decode){
                    a <<= a + 1;
                    purpose = a + 1;
                }
            }

            pip(decode){
                b <<= a;
            }

            seq{
                c <<= c + 1;
                c <<= c + 1;
                c <<= c + 1;
                c <<= c + 1;
                fetch.hold_slave();
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test51.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test51.vcd";


    class sim51 :public SimAutoInterface{
    public:

        test_sim_mod51* _md;

        sim51(test_sim_mod51* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // ////// skip first zync State
            con_next_cycle(1);
            for (int i = 1; i < 5; i++){
                con_end_cycle();
                test_and_print("check a equal to " + std::to_string(i), ull(_md->a), i);
                test_and_print("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
                con_next_cycle(1);
            }
            ///////////////////////////
            con_end_cycle();
            test_and_print(" holding part check a equal to " + std::to_string(4), ull(_md->a), 4);
            test_and_print(" holding part check b equal to " + std::to_string(4), ull(_md->b), 4);
            con_next_cycle(1);
            for (int i = 5; i < 10; i++){
                con_end_cycle();
                test_and_print("check a equal to " + std::to_string(i), ull(_md->a), i);
                test_and_print("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
                con_next_cycle(1);
            }

        }

    };


    class Sim51TestEle: public AutoTestEle{
    public:
        explicit Sim51TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod51, 1);
            start_model_kathryn();
            sim51 simulator((test_sim_mod51*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim51TestEle ele51(51);
}