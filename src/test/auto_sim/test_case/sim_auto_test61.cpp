//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod61: public Module{
    public:

        m_reg(a, 32);
        m_reg(b, 32);

        explicit test_sim_mod61(int x){}

        void flow() override{
            ///// test priority of the zif block

            seq{
                a <<= 0;
                par{
                    SET_CLK_MODE2NEG_EDGE();
                    b <<= 48;
                    SET_CLK_MODE2DEF();
                    a <<= a + b;
                }
            }


        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test61.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test61.vcd";


    class sim61 :public SimAutoInterface{
    public:

        test_sim_mod61* _md;

        sim61(test_sim_mod61* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_next_cycle(2);
            test_and_print("a value updated from neg_edge b:" + std::to_string(48)  , ull(_md->a), 48);
        }

    };


    class Sim61TestEle: public AutoTestEle{
    public:
        explicit Sim61TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod61, 1);
            start_model_kathryn();
            sim61 simulator((test_sim_mod61*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim61TestEle ele61(61);
}