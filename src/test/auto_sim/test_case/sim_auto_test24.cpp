//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod24: public Module{
    public:

        m_reg (i, 6);
        m_reg (k, 8);
        m_wire(b, 6);
        m_reg(c, 32);
        m_mem(storage, 48, 8);


        explicit test_sim_mod24(int x): Module(){}

        void flow() override{

            seq{
                i = 0;
                cwhile(i < 48){
                    par {
                        storage[i] = k;
                        i          = i + 1;
                        k          = k + 1;
                    }
                }
                i = 0;
                cwhile(i < 48){
                    par {
                        i = i + 1;
                        b = storage[i];
                    }
                }
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test24.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test24.vcd";


    class sim24 :public SimAutoInterface{
    public:

        test_sim_mod24* _md;

        sim24(test_sim_mod24* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            con_next_cycle(50);
            for (int i =1; i < 48; i++){
                test_and_print("check_mem with new system : " + std::to_string(i), ull(_md->storage.at(i)), i);
                con_end_cycle();
                test_and_print("check_mem wire assignment : " + std::to_string(i), ull(_md->b), i);
                con_next_cycle(1);
            }
//            for(int i = 0; i < 48; i++){
//                ValRep test_val = NumConverter::create_val_rep(6, i);
//
//                test_and_print("check_mem : " + std::to_string(i), _md->b.sv(), test_val);
//                con_next_cycle(1);
//            }
        }

    };


    class Sim24TestEle: public AutoTestEle{
    public:
        explicit Sim24TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod24, 1);
            start_model_kathryn();
            sim24 simulator((test_sim_mod24*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim24TestEle ele24(24);
}