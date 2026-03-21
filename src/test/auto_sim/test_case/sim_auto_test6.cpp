//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod6: public Module{
    public:
        bool test_auto_skip = false;
        m_val(bnk, 32, 48);
        m_val(zero, 32, 0);
        m_reg(a , 32);
        m_reg(b , 32);

        explicit test_sim_mod6(bool test_auto_skip): Module(){}

        void flow() override{
            Val* value_toused = test_auto_skip ? &zero : &bnk;

            seq{
                a <<= bnk;
                b <<= zero;
                cwhile(a > zero){
                    m_val(one, 32, 1);
                    a <<= a - one;
                }
                b <<= bnk;
            }

        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test6.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test6.vcd";


    class sim6 :public SimAutoInterface{
    public:

        test_sim_mod6* _md;

        sim6(test_sim_mod6* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              100,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                               prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                               sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            inc_cycle(54);

            sim {
                ull test_val = 48;
                test_and_print("check end loop", (ull)_md->b, test_val);
            };

        }

        void sim_driven() override{
            inc_cycle(2);

            sim {
                //_md->iv.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
            };
            inc_cycle(1);
        }

    };


    class Sim6TestEle: public AutoTestEle{
    public:
        explicit Sim6TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod6, 1);
            start_model_kathryn();
            sim6 simulator((test_sim_mod6*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim6TestEle ele6(6);

    ///sim2 test_case2;




}