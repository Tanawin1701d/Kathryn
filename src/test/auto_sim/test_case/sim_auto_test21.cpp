//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod21: public Module{
    public:
        m_reg(a, 32);
        m_wire(result, 1);

        explicit test_sim_mod21(int x): Module(){}

        void flow() override{

            seq {
                a <<= 16;
                sy_wait(a);
                result = 1;
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test21.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test21.vcd";


    class sim21 :public SimAutoInterface{
    public:

        test_sim_mod21* _md;

        sim21(test_sim_mod21* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              300,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            for(int i = 0; i < 17; i++){
                if ((i % 4) == 0) {
                    con_end_cycle();
                    test_and_print("test_result is waiting", ull(_md->result), 0);
                }
                con_next_cycle(1);
            }
            con_end_cycle();
            test_and_print("test_result is waiting set", ull(_md->result), 1);

        }

    };


    class Sim21TestEle: public AutoTestEle{
    public:
        explicit Sim21TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod21, 1);
            start_model_kathryn();
            sim21 simulator((test_sim_mod21*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim21TestEle ele21(21);
}