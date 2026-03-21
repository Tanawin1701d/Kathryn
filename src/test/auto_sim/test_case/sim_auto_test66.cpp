//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod66: public Module{
    public:


        m_reg(  a, 32);
        m_reg(  b, 32);
        m_reg(r1, 32);
        m_reg(r2, 32);
        m_reg(res, 32);
        m_wire(fin, 1);


        explicit test_sim_mod66(int x){}

        void flow() override{

            a.make_reset_event(4);
            b.make_reset_event(1);

            ///// test priority of the zif block
            pip_stream{
                par{
                    a <<= a * 2;
                    b <<= b * 2;
                }
                seq{
                    par{
                        sqrt_int(a, r1);
                        sqrt_int(b, r2);
                    }
                    par{
                        res <<= r1 * r2;
                        fin = 1;
                    }
                }

            }
        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test66.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test66.vcd";


    class sim66 :public SimAutoInterface{
    public:

        test_sim_mod66* _md;

        sim66(test_sim_mod66* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_next_cycle(8);
            test_and_print("res", ull(_md->res), 3);
            con_next_cycle(6);
            test_and_print("res", ull(_md->res), 8);
            con_next_cycle(7);
            test_and_print("res", ull(_md->res), 15);

        }

    };


    class Sim66TestEle: public AutoTestEle{
    public:
        explicit Sim66TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod66, 1);
            start_model_kathryn();
            sim66 simulator((test_sim_mod66*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim66TestEle ele66(66);
}
