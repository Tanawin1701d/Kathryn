//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod35: public Module{
    public:
        m_reg(a, 8);
        m_reg(b, 8);


        explicit test_sim_mod35(int x): Module(){}

        void flow() override{

            seq{
                cwhile(a < 16){
                    a <<= a + 1;
                    for (int i = 0; i < 2; i++){
                        b <<= a + 2;
                    }
                }
            }
        }

    };

    class sim35 :public SimAutoInterface{
    public:

        test_sim_mod35* _md = nullptr;

        sim35(test_sim_mod35* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,100,
                                                                                  prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                                                                  prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
        _md(md)
        {}
        void sim_assert() override{}
        void sim_driven() override{}
    };

    class Sim35TestEle: public AutoTestEle{
    public:
        explicit Sim35TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod35, 1);
            start_model_kathryn();
            sim35 simulator((test_sim_mod35*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim35TestEle ele35(35);

}