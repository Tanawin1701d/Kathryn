//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod16: public Module{
    public:

        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_reg(d, 8);

        m_reg(result, 32);


        explicit test_sim_mod16(int x): Module(){}

        void flow() override{

            seq{
                a <<= 1;
                b(2) <<= 1;
                c(3) <<= 1;
                d(4) <<= 1;
                result   <<= g(d(0, 4), d(5,8),d(4),c,b,a);
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test16.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test16.vcd";


    class sim16 :public SimAutoInterface{
    public:

        test_sim_mod16* _md;

        sim16(test_sim_mod16* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


              inc_cycle(7);
//
            sim{
                ull test_val = 0x01080401;
                test_and_print("nest check with slice", (ull)_md->result, test_val);
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


    class Sim16TestEle: public AutoTestEle{
    public:
        explicit Sim16TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod16, 1);
            start_model_kathryn();
            sim16 simulator((test_sim_mod16*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim16TestEle ele16(16);
}