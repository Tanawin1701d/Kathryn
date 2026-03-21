//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod10: public Module{
    public:
        bool test_auto_skip = false;

        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(result, 8);
        m_val(result_cna,  8, 255);
        m_val(result_cnb,  8, 128);
        m_val(result_cnb2, 8, 129);
        m_val(result_cnc, 8, 20);

        m_reg(inner_a, 8);
        m_reg(inner_b, 8);

        m_val(inner_val_a, 8, 2);
        m_val(inner_val_b, 8, 3);



        explicit test_sim_mod10(bool test_auto_skip): Module(){}

        void flow() override{

            seq {
                m_val(bnk, 8, 48);
                m_val(akb, 8, 49);
                inner_a <<= inner_val_a;
                inner_b <<= inner_val_b;
                a <<= bnk;
                b <<= akb;

                sif(a > b){
                    result <<= result_cna;
                }
                selif(a < b){
                    cif(inner_a > inner_b){
                        result <<= result_cnb;
                    }celse{
                        result <<= result_cnb2;
                    }
                }
                selse{
                    result <<= result_cnc;
                }

            }

        }

    };

    class sim10 :public SimAutoInterface{
    public:

        test_sim_mod10* _md;

        sim10(test_sim_mod10* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                              sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            // inc_cycle(8);
            //
            // sim {
            //     ull test_val = 129;
            //     test_and_print("check cifelse in sifelse", (ull)_md->result, test_val);
            // };

        }

        void sim_driven() override{
            inc_cycle(2);

            sim {
                //_md->iv.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
            };
            inc_cycle(1);
        }

    };


    class Sim10TestEle: public AutoTestEle{
    public:
        explicit Sim10TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod10, 1);
            start_model_kathryn();
            sim10 simulator((test_sim_mod10*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim10TestEle ele10(10);

    ///sim2 test_case2;

}