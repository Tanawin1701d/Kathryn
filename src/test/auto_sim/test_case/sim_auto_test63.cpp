//
// Created by tanawin on 17/9/2568.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    ////// simple pipeline
    class test_sim_mod63: public Module{
    public:

        m_reg(switch_val, 3);
        m_reg(sub_check, 1);
        m_reg(a, 32);
        m_reg(b, 32);

        explicit test_sim_mod63(int x){}

        void flow() override{
            ///// test priority of the zif block

            seq{
                par{
                    switch_val <<= 4;
                    sub_check  <<= 1;
                }

                par{
                    ztate(switch_val){
                        zcase(0b100){
                            a <<=  9;
                            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
                            b <<= 48;
                            SET_ASM_PRI_TO_AUTO();
                            b <<= 24;
                        }
                        zcase(0b001){
                            a <<= 10;
                            b <<= 107;
                            b <<= 108;
                        }
                        zcasedef{
                            b <<= 404;
                        }
                    }
                }
            }


        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test63.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test63.vcd";


    class sim63 :public SimAutoInterface{
    public:

        test_sim_mod63* _md;

        sim63(test_sim_mod63* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_next_cycle(1);
            test_and_print("switch_val before active", ull(_md->switch_val),   4 );
            test_and_print("sub_check before active",  ull(_md->sub_check),    1 );
            test_and_print("a before active",         ull(_md->a),           0 );
            test_and_print("b before active",         ull(_md->b),           0 );
            con_next_cycle(1);
            test_and_print("a after active",         ull(_md->a),  9);
            test_and_print("b after active",         ull(_md->b), 48);
        }

    };


    class Sim63TestEle: public AutoTestEle{
    public:
        explicit Sim63TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod63, 1);
            start_model_kathryn();
            sim63 simulator((test_sim_mod63*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim63TestEle ele63(63);
}