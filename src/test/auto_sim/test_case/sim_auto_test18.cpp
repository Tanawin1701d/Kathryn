//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod18: public Module{
    public:

        m_reg(i, 6);
        m_reg(k, 8);
        m_reg(b, 6);
        m_reg(c, 32);
        m_mem(storage, 48, 8);


        explicit test_sim_mod18(int x): Module(){}

        void flow() override{

            seq{
                i <<= 0;
                cwhile(i < 47){
                    /////// max in this scope is 47
                    par {
                        storage[i] <<= k;
                        i <<= i + 1;
                        k <<= k + 1;
                    }
                }
                i <<= 0;
                i <<= 0;
                cwhile(i < 47){
                    par {
                        i <<= i + 1;
                        b <<= storage[i];
                    }
                }
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test18.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test18.vcd";


    class sim18 :public SimAutoInterface{
    public:

        test_sim_mod18* _md;

        sim18(test_sim_mod18* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            con_next_cycle(52);
            for (int i =0; i < 48; i++){
                test_and_print("check_mem with new system : " + std::to_string(i), (ull)(_md->storage.at(i)), i);
            }
            for(int i = 0; i < 48; i++){
                ull test_val = i;

                test_and_print("check_mem : " + std::to_string(i), (ull)_md->b, test_val);
                con_next_cycle(1);
            }
        }

    };


    class Sim18TestEle: public AutoTestEle{
    public:
        explicit Sim18TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod18, 1);
            start_model_kathryn();
            sim18 simulator((test_sim_mod18*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim18TestEle ele18(18);
}