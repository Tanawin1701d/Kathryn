//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod9: public Module{
    public:
        bool test_auto_skip = false;

        m_val(bnk, 32, 48);
        m_val(akb, 32, 48);
        m_val(end_const, 112, 0b1111111111111111);
        m_reg(cnt, 32);
        m_reg(frd, 32);
        m_reg(end, 3);

        m_val(one, 32,  1);


        explicit test_sim_mod9(bool test_auto_skip): Module(){}

        void flow() override{

            seq {
                par {
                    swhile(cnt < bnk) {
                        cnt <<= cnt + one;
                    }
                    frd <<= akb;

                }
                end <<= end_const;
            }

        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test9.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test9.vcd";


    class sim9 :public SimAutoInterface{
    public:

        test_sim_mod9* _md;

        sim9(test_sim_mod9* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                              sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            inc_cycle(101);

            sim {
                ull test_val = 0b111;
                test_and_print("check end loop with syn_node", (ull)_md->end, test_val);
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



    class Sim9TestEle: public AutoTestEle{
    public:
        explicit Sim9TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod9, 1);
            start_model_kathryn();
            sim9 simulator((test_sim_mod9*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

#ifndef NOTEXCEED64

    Sim9TestEle ele9(9);


#endif
    ///sim2 test_case2;




}