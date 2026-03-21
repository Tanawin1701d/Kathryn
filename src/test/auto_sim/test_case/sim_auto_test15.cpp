//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod15: public Module{
    public:

        m_reg(i, 6);
        m_reg(k, 8);
        m_reg(b, 6);
        m_reg(c, 32);
        m_mem(storage, 48, 8);


        explicit test_sim_mod15(int x): Module(){}

        void flow() override{

            seq{
                i <<= 0;
                cwhile(i < 47){
                    /////// max in this scope is 47
                    par {
                        storage[i] <<= k;
                        i <<= i + 1;
                        k <<= k + 1;
                    }
                }
                i <<= 0;
                i <<= 0;
                cwhile(i < 47){
                    par {
                        i <<= i + 1;
                        b <<= storage[i];
                    }
                }
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test15.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test15.vcd";


    class sim15 :public SimAutoInterface{
    public:

        test_sim_mod15* _md;

        sim15(test_sim_mod15* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        int i = 0;
        void sim_assert() override{


             inc_cycle(54);



             for ( int j = 0; j < 48; j++ ){
                sim{
                    ull test_val = i;
                    test_and_print("check mem : " + std::to_string(i), (ull)_md->b, test_val);
                    i++;
                };
                inc_cycle(1);
             }

        }

        void sim_driven() override{
            inc_cycle(2);

//            sim {
//                _md->a.sv() = NumConverter::cvt_str_to_val_rep(8,  2);
//                _md->b.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
//            };

        }

    };


    class Sim15TestEle: public AutoTestEle{
    public:
        explicit Sim15TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod15, 1);
            start_model_kathryn();
            sim15 simulator((test_sim_mod15*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim15TestEle ele15(15);
}