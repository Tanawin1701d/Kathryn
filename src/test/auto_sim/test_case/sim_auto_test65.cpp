//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod65: public Module{
    public:


        m_reg(  a, 32);
        m_reg(  b, 32);
        m_reg(  c, 32);
        m_reg(  d, 32);
        m_reg(res, 32);

        explicit test_sim_mod65(int x){}

        void flow() override{
            ///// test priority of the zif block
            pip_stream{
                seq{
                    a <<= a + 1;
                    sy_wait(3);
                    b <<= a;
                }

                seq{
                    c <<= b;
                    sy_wait(6);
                    d <<= c;
                }
            }
        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test65.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test65.vcd";


    class sim65 :public SimAutoInterface{
    public:

        test_sim_mod65* _md;

        sim65(test_sim_mod65* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            con_next_cycle(2);
            test_and_print("a", ull(_md->a), 1);
            test_and_print("b", ull(_md->b), 0);
            test_and_print("c", ull(_md->c), 0);
            test_and_print("d", ull(_md->d), 0);
            con_next_cycle(4);
            test_and_print("a", ull(_md->a), 1);
            test_and_print("b", ull(_md->b), 1);
            test_and_print("c", ull(_md->c), 0);
            test_and_print("d", ull(_md->d), 0);
            con_next_cycle(1);
            test_and_print("a", ull(_md->a), 2);
            test_and_print("b", ull(_md->b), 1);
            test_and_print("c", ull(_md->c), 1);
            test_and_print("d", ull(_md->d), 0);
            con_next_cycle(4);
            test_and_print("a", ull(_md->a), 2);
            test_and_print("b", ull(_md->b), 2);
            test_and_print("c", ull(_md->c), 1);
            test_and_print("d", ull(_md->d), 0);
            con_next_cycle(4);
            test_and_print("a", ull(_md->a), 3);
            test_and_print("b", ull(_md->b), 2);
            test_and_print("c", ull(_md->c), 2);
            test_and_print("d", ull(_md->d), 1);
            con_next_cycle(4);
            test_and_print("a", ull(_md->a), 3);
            test_and_print("b", ull(_md->b), 3);
            test_and_print("c", ull(_md->c), 2);
            test_and_print("d", ull(_md->d), 1);
            con_next_cycle(4);
            test_and_print("a", ull(_md->a), 4);
            test_and_print("b", ull(_md->b), 3);
            test_and_print("c", ull(_md->c), 3);
            test_and_print("d", ull(_md->d), 2);
        }

    };


    class Sim65TestEle: public AutoTestEle{
    public:
        explicit Sim65TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod65, 1);
            start_model_kathryn();
            sim65 simulator((test_sim_mod65*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim65TestEle ele65(65);
}
