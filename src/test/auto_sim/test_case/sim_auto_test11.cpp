//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod11: public Module{
    public:
        bool test_auto_skip = false;

        m_reg(a, 8);
        m_reg(b, 8);
        m_val(cond, 3, 2);
        m_val(one, 8, 1);
        m_val(two, 8, 2);
        m_val(three, 8, 3);

        explicit test_sim_mod11(bool test_auto_skip): Module(){}

        void flow() override{

            cwhile(cond){
                zif(a > b){
                    a <<= a + one;
                }
                zelif(a < b){
                    a <<= a + two;
                    zif(a > b){
                        b <<= b - one;
                    }zelse{
                        b <<= b - two;
                    }
                }
            }

        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test11.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test11.vcd";


    class sim11 :public SimAutoInterface{
    public:

        test_sim_mod11* _md;

        sim11(test_sim_mod11* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                              sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            inc_cycle(3);
            inc_cycle(5);
            sim {
                ull test_val = 2 + 6 * 2;
                test_and_print("check base line function", (ull)_md->a, test_val);
            };
            sim{
                ull test_val = 48 - 6 * 2;
                test_and_print("check bascheck base line functione line function", (ull)_md->b, test_val);
            };

        }

        void sim_driven() override{
            inc_cycle(2);

            sim {
                _md->a = 2;
                _md->b = 48;
            };

        }

    };


    class Sim11TestEle: public AutoTestEle{
    public:
        explicit Sim11TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod11, 1);
            start_model_kathryn();
            sim11 simulator((test_sim_mod11*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim11TestEle ele11(11);

    ///sim2 test_case2;

}