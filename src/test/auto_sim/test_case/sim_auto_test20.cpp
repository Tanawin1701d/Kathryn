//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod20: public Module{
    public:
        m_reg(a, 32);
        m_wire(result, 1);

        explicit test_sim_mod20(int x): Module(){}

        void flow() override{

            seq {
                par{
                    seq {
                        sc_wait(a == 48);
                        result = 1;
                    }
                    cwhile(a <= 50){
                        a <<= a + 1;
                    }
                }


            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test20.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test20.vcd";


    class sim20 :public SimAutoInterface{
    public:

        test_sim_mod20* _md;

        sim20(test_sim_mod20* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              300,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            for(int i = 0; i <= 48; i++){
                if ((i % 10) == 0) {
                    con_end_cycle();
                    test_and_print("test_result is waiting", ull(_md->result), 0);
                }
                con_next_cycle(1);
            }
            con_end_cycle();
            test_and_print("test_result is waiting set", ull(_md->result), 1);

        }

    };


    class Sim20TestEle: public AutoTestEle{
    public:
        explicit Sim20TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod20, 1);
            start_model_kathryn();
            sim20 simulator((test_sim_mod20*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim20TestEle ele20(20);
}