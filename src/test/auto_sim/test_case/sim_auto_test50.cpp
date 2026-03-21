//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod50: public Module{
    public:
        m_reg(a, 32);
        m_reg(b, 32);
        m_reg(c, 32);
        SyncMeta fetch{"fetch"};
        SyncMeta decode{"decode"};


        explicit test_sim_mod50(int x){}

        void flow() override{
            a.make_reset_event();

            pip(fetch){ auto_sync
                zync(decode){
                    a <<= a + 1;
                }
            }

            pip(decode){
                b <<= a;
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test50.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test50.vcd";


    class sim50 :public SimAutoInterface{
    public:

        test_sim_mod50* _md;

        sim50(test_sim_mod50* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            ////// skip first zync State
            con_next_cycle(1);
            for (int i = 1; i < 5; i++){
                con_end_cycle();
                test_and_print("check a equal to " + std::to_string(i), ull(_md->a), i);
                test_and_print("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
                con_next_cycle(1);
            }

        }

    };


    class Sim50TestEle: public AutoTestEle{
    public:
        explicit Sim50TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod50, 1);
            start_model_kathryn();
            sim50 simulator((test_sim_mod50*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim50TestEle ele50(50);
}