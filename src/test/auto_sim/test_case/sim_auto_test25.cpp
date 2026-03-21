//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod25: public Module{
    public:

        box(my_type){
            init_box(my_type);
            m_reg(test1, 8);
            m_reg(test2, 8);

            box(nex_type){
                init_box(nex_type);
                m_reg(testn, 8);
            };

            m_box(n, nex_type);
        };

        m_box(a, my_type);
        m_box(b, my_type);

        my_type* ccccc;

        explicit test_sim_mod25(int x): Module(){}

        void flow() override{

            seq{
                a.test1   <<= 48;
                a.test2   <<= 50;
                a.n.testn <<= 99;
                b         <<= a;
                m_box(c, my_type);
                ccccc = &c;
                c         = a;
            }


        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test25.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test25.vcd";


    class sim25 :public SimAutoInterface{
    public:

        test_sim_mod25* _md;

        sim25(test_sim_mod25* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{

            con_next_cycle(4);
            test_and_print("check bundle assign b -> test1", ull(_md->b.test1),48);
            test_and_print("check bundle assign b -> test2", ull(_md->b.test2),50);
            test_and_print("check bundle assign b -> testn", ull(_md->b.n.testn),99);
            test_and_print("check bundle assign c -> test1", ull(_md->ccccc->test1),48);
            test_and_print("check bundle assign c -> test2", ull(_md->ccccc->test2),50);
            test_and_print("check bundle assign c -> testn", ull(_md->ccccc->n.testn),99);
//            for(int i = 0; i < 48; i++){
//                ValRep test_val = NumConverter::create_val_rep(6, i);
//
//                test_and_print("check_mem : " + std::to_string(i), _md->b.sv(), test_val);
//                con_next_cycle(1);
//            }
        }

    };


    class Sim25TestEle: public AutoTestEle{
    public:
        explicit Sim25TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod25, 1);
            start_model_kathryn();
            sim25 simulator((test_sim_mod25*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim25TestEle ele25(25);
}