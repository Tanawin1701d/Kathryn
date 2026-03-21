//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod22: public Module{
    public:
        m_reg(a, 32);
        m_reg(m, 32);
        m_reg(b, 32);

        m_val(c, 8, 8);
        m_val(d, 32, 10);

        m_wire(i, 6);
        m_mem(my_st, 48, 8);
        m_wire(my_st_out, 8);
        m_wire(my_st_out_old, 8);


        explicit test_sim_mod22(int x): Module(){}

        void flow() override{

            g(a,m,b) <<= g(c,d);
            m(9) <<= 1;
            /**value*/
            i          = 4;
            my_st[i]    <<= c;
            my_st_out    = my_st[i];
            my_st_out_old = my_st[0];

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test22.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test22.vcd";


    class sim22 :public SimAutoInterface{
    public:

        test_sim_mod22* _md;

        sim22(test_sim_mod22* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              300,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                             _md(md)
        {}

        void describe_con() override{
            test_and_print("test dry nest", ull(_md->m), (1 << 9) + 8);
            con_end_cycle();
            test_and_print("test mem_and_wire dummy", ull(_md->my_st_out_old), 0);
            test_and_print("test mem_and_wire fill" , ull(_md->my_st_out), 8);
        }



    };


    class Sim22TestEle: public AutoTestEle{
    public:
        explicit Sim22TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod22, 1);
            start_model_kathryn();
            sim22 simulator((test_sim_mod22*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };
#ifndef NOTEXCEED64
    Sim22TestEle ele22(22);
#endif

}