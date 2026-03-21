//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod64: public Module{
    public:


        m_reg(  a, 32);
        m_reg(res, 32);

        explicit test_sim_mod64(int x){}

        void flow() override{
            ///// test priority of the zif block

            seq{
                a <<= 40;
                res <<= sqrt_int(a);
            }


        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test64.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test64.vcd";


    class sim64 :public SimAutoInterface{
    public:

        test_sim_mod64* _md;

        sim64(test_sim_mod64* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            con_next_cycle(8);
            test_and_print("sqrt Val", ull(_md->res), 6);
        }

    };


    class Sim64TestEle: public AutoTestEle{
    public:
        explicit Sim64TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod64, 1);
            start_model_kathryn();
            sim64 simulator((test_sim_mod64*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim64TestEle ele64(64);
}
