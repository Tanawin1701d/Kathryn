//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"



namespace kathryn{

    class test_sim_mod33: public Module{
    public:
        m_reg(a, 32);
        m_reg(c, 32);
        m_wire(x, 6);

        explicit test_sim_mod33(int x): Module(){}

        void flow() override{
            seq{
                a <<= 1;
                cdowhile(c <= 5){
                    par {
                        x = c;
                        c <<= c + 1;
                        /////sbreak_con(c == 2);
                    }
                }
            }
        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test33.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test33.vcd";


    class sim33 :public SimAutoInterface{
    public:

        test_sim_mod33* _md;

        sim33(test_sim_mod33* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                                 prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                                _md(md)
        {}

        void describe_con() override{
            for (int i = 0; i < 2; i++){
                con_end_cycle();
                test_and_print("dowhile_pre", ull(_md->x),0);
                con_next_cycle(1);
            }
            for(int i = 1; i <= 6; i++){
                con_end_cycle();
                test_and_print("dowhile_val", ull(_md->x),i);
                con_next_cycle(1);
            }
            for (int i = 0; i < 2; i++){
                con_end_cycle();
                test_and_print("dowhile_post", ull(_md->x),0);
                con_next_cycle(1);
            }
        }
    };


    class Sim33TestEle: public AutoTestEle{
    public:
        explicit Sim33TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod33, 1);
            start_model_kathryn();
            sim33 simulator((test_sim_mod33*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim33TestEle ele33(33);
}