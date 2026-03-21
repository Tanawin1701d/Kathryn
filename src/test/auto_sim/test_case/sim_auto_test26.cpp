//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod26: public Module{
    public:

        m_reg(a, 8);
        m_val(b, 8, 48);
        m_reg(c, 1);
        m_wire(is, 1);

        explicit test_sim_mod26(int x): Module(){}

        void flow() override{

            cwhile( (c == 1) & (a < 48) ){
                seq {
                    intr_start(is);
                    c = 1;
                    a <<= a + 1;
                }
            }

            seq{
                sy_wait(5);
                is = 1;
            }


        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test26.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test26.vcd";


    class sim26 :public SimAutoInterface{
    public:

        test_sim_mod26* _md;

        sim26(test_sim_mod26* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_next_cycle(6);
            test_and_print("check intr seq c start", ull(_md->c),1);
            con_next_cycle(2);
            test_and_print("check intr seq a start", ull(_md->a),1);
            con_next_cycle(2);
            test_and_print("check intr a start", ull(_md->a),2);


        }

    };


    class Sim26TestEle: public AutoTestEle{
    public:
        explicit Sim26TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod26, 1);
            start_model_kathryn();
            sim26 simulator((test_sim_mod26*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim26TestEle ele26(26);
}