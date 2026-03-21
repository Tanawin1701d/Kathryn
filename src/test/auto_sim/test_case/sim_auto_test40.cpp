//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod40: public Module{
    public:

        m_reg (i, 6);
        m_reg (k, 8);
        m_wire(b, 6);
        m_reg(c, 32);
        m_mem(storage, 48, 8);


        explicit test_sim_mod40(int x): Module(){}

        void flow() override{

            i.mark_sv("my_i");
            b.mark_sv("my_b");
            storage.mark_sv("my_mem");

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

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test40.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test40.vcd";


    class sim40 :public SimAutoInterface{
    public:

        test_sim_mod40* _md;

        sim40(test_sim_mod40* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
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


    class Sim40TestEle: public AutoTestEle{
    public:
        explicit Sim40TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod40, 1);
            start_model_kathryn();
            sim40 simulator((test_sim_mod40*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim40TestEle ele40(40);
}