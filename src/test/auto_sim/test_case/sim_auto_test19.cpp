//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod19: public Module{
    public:
        m_reg(a, 32);
        m_reg(b, 32);
        m_reg(c, 32);
        m_wire(d, 32);
        m_reg(end, 1);


        explicit test_sim_mod19(int x): Module(){}

        void flow() override{

            seq {
                cwhile(a <= 48) {
                    seq {
                        b <<= 5;
                        d = 6;
                        a <<= a + 1;
                        cif(a == 16) {
                            par {
                                sbreak;
                                seq{
                                    b <<= 15;
                                }
                            }
                        }
                        d = 7;
                    }
                }
                end <<= 1;
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test19.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test19.vcd";


    class sim19 :public SimAutoInterface{
    public:

        test_sim_mod19* _md;

        sim19(test_sim_mod19* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              300,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            for(int i = 0; i < (17*4); i++){

                if ((i % 10) == 0) {
                    test_and_print("test_end is not set", ull(_md->end), 0);
                }
                con_next_cycle(1);
            }
            con_next_cycle(1);
            test_and_print("test End is set", ull(_md->end), 1);
            test_and_print("test B if break is used", ull(_md->b), 15);
            con_next_cycle(1);
        }

    };


    class Sim19TestEle: public AutoTestEle{
    public:
        explicit Sim19TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod19, 1);
            start_model_kathryn();
            sim19 simulator((test_sim_mod19*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim19TestEle ele19(19);
}