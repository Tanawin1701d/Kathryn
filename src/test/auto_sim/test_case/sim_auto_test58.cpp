//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod58: public Module{
    public:

        m_reg(a, 32);
        m_reg(b, 32);

        explicit test_sim_mod58(int x){}

        void flow() override{
            ///// test priority of the zif block

            zif (a < 16){
                a <<= a + 3;
                a <<= a + 2;
                zif (a < 8){
                    a <<= a + 1;
                }
            }zelif(a < 32){
                zif (a < 24){
                    a <<= 24;
                }zelse{
                    a <<= 48;
                    b <<= 48;
                }
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test58.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test58.vcd";


    class sim58 :public SimAutoInterface{
    public:

        test_sim_mod58* _md;

        sim58(test_sim_mod58* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            // // ////// skip first zync State
            // con_next_cycle(1);
            // con_end_cycle();
            for (int i = 2; i <= 8; i++){
                test_and_print("a value " + std::to_string(i)  , ull(_md->a), i);
                test_and_print("b value " + std::to_string(i)  , ull(_md->b), 0);
                con_next_cycle(1);
                con_end_cycle();
            }
            for (int i = 10; i <= 16; i+=2){
                test_and_print("a value " + std::to_string(i)  , ull(_md->a), i);
                test_and_print("b value " + std::to_string(i)  , ull(_md->b), 0);
                con_next_cycle(1);
                con_end_cycle();
            }
            test_and_print("a value " + std::to_string(24)  , ull(_md->a), 24);
            test_and_print("b value " + std::to_string(0)  , ull(_md->b), 0);
            con_next_cycle(1);
            con_end_cycle();
            test_and_print("a value " + std::to_string(48)  , ull(_md->a), 48);
            test_and_print("b value " + std::to_string(48)  , ull(_md->b), 48);
        }

    };


    class Sim58TestEle: public AutoTestEle{
    public:
        explicit Sim58TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod58, 1);
            start_model_kathryn();
            sim58 simulator((test_sim_mod58*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim58TestEle ele58(58);
}