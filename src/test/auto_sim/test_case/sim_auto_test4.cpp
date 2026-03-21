//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod4: public Module{
    public:
        m_reg(a0, 8);
        m_reg(b0, 8);
        /** lane1*/
        m_reg(a1, 8);
        m_reg(b1, 8);
        m_reg(c1, 8);
        m_reg(d1, 8);
        /** lane2*/
        m_reg(a2, 8);
        m_reg(b2, 8);
        m_reg(c2, 8);

        m_val(iv,   8,48);
        m_val(iv2,  8,64);
        m_val(zero, 8, 0);


        explicit test_sim_mod4(int x): Module(){}

        void flow() override{
            seq {
                a0 <<= iv;
                par {
                    seq {
                        a1 <<= iv;
                        b1 <<= a1;
                        c1 <<= b1;
                        d1 <<= c1;
                    }
                    seq {
                        a2 <<= iv2;
                        b2 <<= a2;
                        c2 <<= b2;
                    }
                }
                b0 <<= iv;
            }

        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test4.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test4.vcd";


    class sim4 :public SimAutoInterface{
    public:

        test_sim_mod4* _md;

        sim4(test_sim_mod4* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              100,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                               prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                               sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            inc_cycle(7);

            sim {
                ull test_val = 48;
                test_and_print("check End Val", (ull)_md->d1, test_val);
            };

            set_cycle(6);

            sim{
                ull test_val = 64;
                test_and_print("check End Val", (ull)_md->c2, test_val);
            };
        }

        void sim_driven() override{
            inc_cycle(2);

            // sim {
            //     _md->iv = 48;
            // };
            inc_cycle(1);
        }

    };


    class Sim4TestEle: public AutoTestEle{
    public:
        explicit Sim4TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod4, 1);
            start_model_kathryn();
            sim4 simulator((test_sim_mod4*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim4TestEle ele4(4);

    ///sim2 test_case2;




}