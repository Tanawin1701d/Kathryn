//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod14: public Module{
    public:

        m_reg(a, 32);
        m_reg(b, 32);
        m_reg(c, 32);

        explicit test_sim_mod14(int x): Module(){}

        void flow() override{

            seq{
                /** we want [3,6]*/
                a(3, 8)  <<= 3 + 16;
                b(9, 13) <<= 3;
                c <<= a(3, 7) + b(9, 13);

            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test14.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test14.vcd";


    class sim14 :public SimAutoInterface{
    public:

        test_sim_mod14* _md;

        sim14(test_sim_mod14* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            inc_cycle(5);

            sim{
                ull test_val = 6;
                test_and_print("check expression on slicing", (ull)_md->c, test_val);
            };

//            sim {
//                ValRep test_val = NumConverter::cvt_str_to_val_rep(67, 0b100, 0);
//                test_and_print("check shifting from slicing", _md->a.sv(), test_val);
//            };
//            sim {
//                ValRep test_val = NumConverter::cvt_str_to_val_rep(8, 2 + 6*2);
//                test_and_print("check base line function", _md->a.sv(), test_val);
//            };
//            sim{
//                ValRep test_val = NumConverter::cvt_str_to_val_rep(8, 48 - 6*2);
//                test_and_print("check bascheck base line functione line function", _md->b.sv(), test_val);
//            };

        }

        void sim_driven() override{
            inc_cycle(2);

//            sim {
//                _md->a.sv() = NumConverter::cvt_str_to_val_rep(8,  2);
//                _md->b.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
//            };

        }

    };


    class Sim14TestEle: public AutoTestEle{
    public:
        explicit Sim14TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod14, 1);
            start_model_kathryn();
            sim14 simulator((test_sim_mod14*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim14TestEle ele14(14);
}