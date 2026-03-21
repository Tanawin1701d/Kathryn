//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod30: public Module{
    public:

        m_reg (a, 8);
        m_wire(is, 1);
        m_reg (b, 8);


        explicit test_sim_mod30(int x): Module(){}

        void flow() override{

            cwhile(true){
                intr_reset(is);
                seq{
                    par{
                        cwhile(a < 48){
                            cif(a(0)){
                                seq {
                                    a = a + 1;
                                    a = a + 1;
                                }
                            }celse{
                                a = a + 2;
                            }
                        }
                        b = b + 1;
                    }
                    a = a + 1;
                }
            }

            seq{
                sy_wait(5);
                par {
                    is = 1;
                }
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test30.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test30.vcd";


    class sim30 :public SimAutoInterface{
    public:

        test_sim_mod30* _md;

        sim30(test_sim_mod30* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

//            con_next_cycle(6);
//            for(int i =1; i <= 48; i++){
//                if (i % 10 == 1)
//                    test_and_print("check intr par a start", ull(_md->a),i);
//                con_next_cycle(1);
//            }

        }

    };


    class Sim30TestEle: public AutoTestEle{
    public:
        explicit Sim30TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod30, 1);
            start_model_kathryn();
            sim30 simulator((test_sim_mod30*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim30TestEle ele30(30);
}