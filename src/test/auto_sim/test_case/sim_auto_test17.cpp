//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod17: public Module{
    public:

        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_reg(d, 8);
        m_reg(e, 8);
        m_reg(f, 8);
        m_reg(h, 8);


        m_reg(result, 32);


        explicit test_sim_mod17(int x): Module(){}

        void flow() override{

            seq{
                a <<= 1;
                b <<= 1;
                g(d,
                  g(f(0,4),e(0,4)),
                  h)(8, 17) <<= 17 + (1 << 8);
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test17.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test17.vcd";


    class sim17 :public SimAutoInterface{
    public:

        test_sim_mod17* _md;

        sim17(test_sim_mod17* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


              inc_cycle(5);
//
            sim{
                ull test_val = 1;
                test_and_print("test recur nest d", (ull)_md->d, test_val);
                test_and_print("test recur nest e", (ull)_md->e, test_val);
                test_and_print("test recur nest f", (ull)_md->f, test_val);
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
            inc_cycle(7);

//            sim {
//                _md->a.sv() = NumConverter::cvt_str_to_val_rep(8,  2);
//                _md->b.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
//            };

        }

    };


    class Sim17TestEle: public AutoTestEle{
    public:
        explicit Sim17TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod17, 1);
            start_model_kathryn();
            sim17 simulator((test_sim_mod17*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim17TestEle ele17(17);
}