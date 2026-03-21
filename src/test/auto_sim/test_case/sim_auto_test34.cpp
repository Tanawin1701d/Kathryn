//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"



namespace kathryn{

    class test_sim_mod34: public Module{
    public:

        std::vector<Reg*> cnts;

        explicit test_sim_mod34(int x): Module(){}

        void flow() override{

            for (int i = 0; i < 200; i++){
                m_reg(a, 8);
                cnts.push_back(&a);
                *cnts[i] <<= *cnts[i] + 1;
            }
        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test34.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test34.vcd";


    class sim34 :public SimAutoInterface{
    public:

        test_sim_mod34* _md;

        sim34(test_sim_mod34* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                                 100,
                                                 prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                                 prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                                _md(md)
        {}

        void describe_con() override{

        }
    };


    class Sim34TestEle: public AutoTestEle{
    public:
        explicit Sim34TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod34, 1);
            start_model_kathryn();
            sim34 simulator((test_sim_mod34*) &d, _simId, prefix, sim_proxy_build_mode);
            auto start = std::chrono::steady_clock::now();
            simulator.sim_start();
            std::cout << TC_GREEN << "--------------------------------" << std::endl;
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
        }

    };

    Sim34TestEle ele34(34);
}