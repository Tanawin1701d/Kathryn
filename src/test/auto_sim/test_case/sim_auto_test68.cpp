//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"
#include "lib/math/math.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod68: public Module{
    public:
        m_reg (a, 32);
        m_wire(b, 32);


        explicit test_sim_mod68(int x){}

        void flow() override{

            par{
                a <<= 48;
                b =   48;
            }




        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test68.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test68.vcd";


    class sim68 :public SimAutoInterface{
    public:

        test_sim_mod68* _md;

        sim68(test_sim_mod68* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_end_cycle();
            test_and_print("check a REG is not set to" + std::to_string(0), ull(_md->a), 0);
            test_and_print("check b REG is not set to" + std::to_string(48), ull(_md->b), 48);
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("check a REG is not set to" + std::to_string(0), ull(_md->a), 48);
            test_and_print("check b REG is not set to" + std::to_string(48), ull(_md->b), 0);


            // ////// skip first zync State
            // con_next_cycle(1);
            // for (int i = 1; i < 5; i++){
            //     con_end_cycle();
            //     test_and_print("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     test_and_print("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     con_next_cycle(1);
            // }
            // ///////////////////////////
            // con_end_cycle();
            // test_and_print(" holding part check a equal to " + std::to_string(4), ull(_md->a), 4);
            // test_and_print(" holding part check b equal to " + std::to_string(4), ull(_md->b), 4);
            // con_next_cycle(1);
            // for (int i = 5; i < 10; i++){
            //     con_end_cycle();
            //     test_and_print("check a equal to " + std::to_string(i), ull(_md->a), i);
            //     test_and_print("check b equal to " + std::to_string(i-1), ull(_md->b), i-1);
            //     con_next_cycle(1);
            // }

        }

    };


    class Sim68TestEle: public AutoTestEle{
    public:
        explicit Sim68TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod68, 1);
            start_model_kathryn();
            sim68 simulator((test_sim_mod68*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim68TestEle ele68(68);
}
