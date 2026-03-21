//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod28: public Module{
    public:

        m_reg(a, 8);
        m_reg(a2, 8);
        m_val(b, 8, 48);
        m_reg(c, 1);
        m_reg(d, 1);
        m_wire(is, 1);

        explicit test_sim_mod28(int x): Module(){}

        void flow() override{

            cwhile(  c == 1 ){
                cif(a < 10){ intr_start(is); ////// it start at cif not inside cif
                    a = a+1;
                }celse{
                    a = a+2;
                };

                sif(a2 < 7){ intr_start(is); ////// it start at cif not inside sif
                    a2 = a2+1;
                }selif(a2 < 23){
                    a2 = a2+2;
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

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test28.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test28.vcd";


    class sim28 :public SimAutoInterface{
    public:

        test_sim_mod28* _md;

        sim28(test_sim_mod28* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_next_cycle(5);
            test_and_print("check intr ifelse trigger c start", ull(_md->c),1);
            con_next_cycle(1);
            test_and_print("check intr par a start",  ull(_md->a ),1);
            test_and_print("check intr par a2 start", ull(_md->a2),0);
            con_next_cycle(1);
            test_and_print("check intr par a start",  ull(_md->a ),1);
            test_and_print("check intr par a2 start", ull(_md->a2),1);
            con_next_cycle(1);
            test_and_print("check intr par a start",  ull(_md->a ),2);
            test_and_print("check intr par a2 start", ull(_md->a2),1);
            con_next_cycle(1);
            test_and_print("check intr par a start",  ull(_md->a ),2);
            test_and_print("check intr par a2 start", ull(_md->a2),2);
            con_next_cycle(1);
            test_and_print("check intr par a start",  ull(_md->a ),3);
            test_and_print("check intr par a2 start", ull(_md->a2),2);
            con_next_cycle(1);
            test_and_print("check intr par a start",  ull(_md->a ),3);
            test_and_print("check intr par a2 start", ull(_md->a2),3);
        }

    };


    class Sim28TestEle: public AutoTestEle{
    public:
        explicit Sim28TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod28, 1);
            start_model_kathryn();
            sim28 simulator((test_sim_mod28*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim28TestEle ele28(28);
}