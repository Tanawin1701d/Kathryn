//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod39: public Module{
    public:

        m_reg(a  , 138);
        m_reg(b  , 138);

        explicit test_sim_mod39(int x): Module(){}

        void flow() override{

            seq{
                a <<= b;
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test39.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test39.vcd";


    class sim39 :public SimAutoInterface{
    public:

        test_sim_mod39* _md;

        sim39(test_sim_mod39* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            _md->b.s(hex(136, "F0000000000000001"));
            con_next_cycle(1);

            test_and_print("a[0]", _md->a.v().get_large_val()[0], 1);
            test_and_print("a[1]", _md->a.v().get_large_val()[1], 15);
            test_and_print("a[2]", _md->a.v().get_large_val()[2], 0);

            // auto x = _md->a.v().get_large_val();
            // std::cout << "get large value" << std::endl;
            // for (ull v: x){ std::cout << v << std::endl; }
            // std::cout << "end get large value" << std::endl;
        }



    };


    class Sim39TestEle: public AutoTestEle{
    public:
        explicit Sim39TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod39, 1);
            start_model_kathryn();
            sim39 simulator((test_sim_mod39*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim39TestEle ele39(39);

}