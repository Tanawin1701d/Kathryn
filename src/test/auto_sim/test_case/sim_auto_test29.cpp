//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod29: public Module{
    public:

        m_reg(a, 8);
        m_reg(a2, 8);
        m_val(b, 8, 48);
        m_reg(c, 1);
        m_reg(d, 1);
        m_wire(is, 1);

        explicit test_sim_mod29(int x): Module(){}

        void flow() override{

            cwhile(  c == 1 ){

                cwhile(true){
                    intr_start(is);
                    seq{
                        cif(a == 48){
                            sbreak;
                        }celse{
                            a = a + 1;
                        }
                    }
                }

            }

            seq{
                sy_wait(5);
                par {
                    is = 1;
                    c  = 1;
                }
            }


        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test29.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test29.vcd";


    class sim29 :public SimAutoInterface{
    public:

        test_sim_mod29* _md;

        sim29(test_sim_mod29* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_next_cycle(6);
            for(int i =1; i <= 48; i++){
                if (i % 10 == 1)
                    test_and_print("check intr par a start", ull(_md->a),i);
                con_next_cycle(1);
            }

        }

    };


    class Sim29TestEle: public AutoTestEle{
    public:
        explicit Sim29TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod29, 1);
            start_model_kathryn();
            sim29 simulator((test_sim_mod29*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim29TestEle ele29(29);
}