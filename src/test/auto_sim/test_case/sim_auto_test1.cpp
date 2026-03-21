//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod: public Module{
    public:
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_val(iv, 8, 0b10101010);

        explicit test_sim_mod(int x): Module(){}

        void flow() override{

            seq{
                a <<= iv;
                b <<= a;
                c <<= b;
                m_val(zero, 8, 0);
                a <<= zero;
                b <<= zero;
                c <<= zero;
                for (int i = 0; i < 4; i++){
                    m_val(x, 8, i+1);
                    a <<= x;
                }
                m_val(y, 8, 3);
                a <<= a << y;
            }

//
//            seq{
//                cif(a < b){
//                    a <<= b;
//                }celse{
//                    b <<= a;
//                }
//            }
//
//            cwhile(a<b){
//                a <<= a + 1;
//            }

        }

    };

    class sim1 :public SimAutoInterface{
    public:

        test_sim_mod* _md = nullptr;

        sim1(test_sim_mod* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,100,
                                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                                                  sim_proxy_build_mode),
        _md(md)
        {}

        void sim_assert() override{
            inc_cycle(13);
            sim {

                test_and_print("check End Val", (ull)_md->a,
                    32);
            };
            sim{
                ull test_val = 32;
                test_and_print("check C Val", (ull)_md->a, test_val);
            };
        }

        void sim_driven() override{
            // inc_cycle(2);
            // for (int i = 0; i < 100; i++) {
            //     sim {
            //         _md->iv = 7;
            //         _md->c  = 7;
            //     };
            //     inc_cycle(1);
            // }
        }
    };

    class Sim1TestEle: public AutoTestEle{
    public:
        explicit Sim1TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod, 1);
            start_model_kathryn();
            sim1 simulator((test_sim_mod*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim1TestEle ele1(1);

}