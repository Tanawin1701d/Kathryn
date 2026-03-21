//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod2: public Module{
    public:
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_val(iv, 8, 7);///0b10101010);

        explicit test_sim_mod2(int x): Module(){}

        void flow() override{

            m_val(bnk, 8, 48);
            m_val(one, 8, 1);
            m_val(two, 8, 2);
            m_val(maxer, 8, 255);
            m_reg(ota, 8);
            m_reg(max_reg, 8);
            m_reg(bw_or_reg, 8);


            par{
                a <<= iv;
                b <<= iv;
                c <<= bnk;
                ota <<= bnk - one;
                max_reg <<= maxer + two;
                bw_or_reg <<= bnk | (one & one);
            }

        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test2.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test2.vcd";


    class sim2 :public SimAutoInterface{
    public:

        test_sim_mod2* _md;

        sim2(test_sim_mod2* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                100,
                                prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                sim_proxy_build_mode),
                                _md(md)
        {}

        void sim_assert() override{


            inc_cycle(3);

            sim {
                ull test_val = 7;
                test_and_print("check End Val", (ull)_md->a, 7);
            };
        }

        void sim_driven() override{
            // inc_cycle(2);
            //
            // sim {
            //     _md->iv = 7;
            // };
            // inc_cycle(1);
        }

    };


    class Sim2TestEle: public AutoTestEle{
    public:
        explicit Sim2TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod2, 1);
            start_model_kathryn();
            sim2 simulator((test_sim_mod2*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim2TestEle ele2(2);

    ///sim2 test_case2;




}