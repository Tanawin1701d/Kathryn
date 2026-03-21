//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"
//#include "carolyne/arch/caro/caro_repo.h"

namespace kathryn{

    class test_sim_mod44: public Module{

        m_reg(i, 10);
        m_reg(j, 10);
        m_reg(result, 1);
    public:
        explicit test_sim_mod44(int x){}

        void flow() override{

            seq{
                par{i = 0; j = 0; result = 0;}
                par{
                    cwhile(i < 4){ mark_join_master
                        i = i + 1;
                    }
                    cwhile(j < 10){j = j + 1;}
                };
                result = 1;
            }
        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test44.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test44.vcd";


    class sim44 :public SimAutoInterface{
    public:

        test_sim_mod44* _md;

        sim44(test_sim_mod44* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{}

    };


    class Sim44TestEle: public AutoTestEle{
    public:
        explicit Sim44TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod44, 1);
            start_model_kathryn();
            sim44 simulator((test_sim_mod44*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim44TestEle ele44(44);
}
