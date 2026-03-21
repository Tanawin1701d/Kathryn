//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod38: public Module{
    public:
        m_val(bnk, 32, 48);
        m_val(zero, 32, 0);
        m_reg(a , 32);
        m_reg(b , 32);

        m_reg(r1, 32);
        m_reg(r2, 32);
        m_reg(rend, 32);

        explicit test_sim_mod38(bool test_auto_skip): Module(){}

        void flow() override{
            seq{
                cwhile(r1 < 20){
                    r1 = r1 + 1;
                    pick{
                        pif(r1 == 8){
                            a <<= 48;
                        }
                        pif(r1 == 16){
                            a <<= 24;
                        }
                        pick_def
                    }
                }
                rend <<= 48;
            }
        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test38.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test38.vcd";


    class sim38 :public SimAutoInterface{
    public:

        test_sim_mod38* _md;

        sim38(test_sim_mod38* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              100,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                               prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            int r1 = 1;
            //////before 8
            for (; r1 <= 8; r1++){
                test_and_print("test_r", ull(_md->r1), r1);
                con_next_cycle(1);
            }
            test_and_print("test_r", ull(_md->r1), 8);
            con_next_cycle(1);
            test_and_print("test_r", ull(_md->r1), 9);
            test_and_print("test_a", ull(_md->a), 48);
            con_next_cycle(1);
            for(r1 = 10; r1 <= 16; r1++){
                test_and_print("test_r", ull(_md->r1), r1);
                con_next_cycle(1);
            }
            test_and_print("test_r", ull(_md->r1), 16);
            con_next_cycle(1);
            test_and_print("test_r", ull(_md->r1), 17);
            test_and_print("test_a", ull(_md->a), 24);
            con_next_cycle(1);

            for (r1 = 18; r1 <= 20; r1++){
                con_next_cycle(1);
            }
            test_and_print("final_ans", ull(_md->rend), 0);
            con_next_cycle(1);
            test_and_print("final_ans", ull(_md->rend), 48);

        }

    };


    class Sim38TestEle: public AutoTestEle{
    public:
        explicit Sim38TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod38, 1);
            start_model_kathryn();
            sim38 simulator( &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim38TestEle ele38(38);

    ///sim2 test_case2;




}