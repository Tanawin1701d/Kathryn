//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"



namespace kathryn{

    class test_sim_mod37: public Module{
    public:
        m_reg(a, 128);
        m_reg(b, 128);
        m_reg(c, 64+32);

        m_reg(r0, 128);
        m_reg(r1, 128);
        m_reg(r2, 128);
        m_reg(r3, 128);

        m_reg(d0, 8);

        explicit test_sim_mod37(int x): Module(){}

        void flow() override{
            seq{
                a(64, 72) <<= 1;
                b(64, 72) <<= 3;
                r0 <<= a & b;
                r1 <<= a | b;
                r2 <<= a ^ b;
                r3 <<= !a;
                c  <<= ~c;
                c  <<= c >> 66;
                c  <<= c << 66;
                d0 <<= a(64, 72);

            }
        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test37.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test37.vcd";


    class sim37 :public SimAutoInterface{
    public:

        test_sim_mod37* _md;

        sim37(test_sim_mod37* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                                 prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
                                _md(md)
        {}

        void describe_con() override{

            con_next_cycle(1);

            ull test_a = ((ValRepBase)_md->a).get_large_val()[1];
            test_and_print("testL_a", test_a, 1);

            con_next_cycle(1);

            ull test_b = ((ValRepBase)_md->b).get_large_val()[1];
            test_and_print("testL_b", test_b, 3);

            con_next_cycle(1);

            ull test_r0 = ((ValRepBase)_md->r0).get_large_val()[1];
            test_and_print("testL_r0", test_r0, 1);

            con_next_cycle(1);

            ull test_r1 = ((ValRepBase)_md->r1).get_large_val()[1];
            test_and_print("testL_r1", test_r1, 3);

            con_next_cycle(1);

            ull test_r2 = ((ValRepBase)_md->r2).get_large_val()[1];
            test_and_print("testL_r2", test_r2, 2);

            con_next_cycle(1);

            ull test_r3 = ((ValRepBase)_md->r3).get_large_val()[1];
            test_and_print("testL_r3", test_r3, 0);

            con_next_cycle(1);

            ull test_ch = ((ValRepBase)_md->c).get_large_val()[1];
            ull test_cl = ((ValRepBase)_md->c).get_large_val()[0];
            test_and_print("testH_c0", test_ch, UINT32_MAX);
            test_and_print("testL_c0", test_cl, UINT64_MAX);

            con_next_cycle(1);

            ull testCH_1 = ((ValRepBase)_md->c).get_large_val()[1];
            ull testCL_1 = ((ValRepBase)_md->c).get_large_val()[0];
            test_and_print("testH_c1", testCH_1, 0);
            test_and_print("testL_c1", testCL_1, 0x3FFFFFFF);

            con_next_cycle(1);

            ull testCH_2 = ((ValRepBase)_md->c).get_large_val()[1];
            ull testCL_2 = ((ValRepBase)_md->c).get_large_val()[0];
            test_and_print("testH_c2", testCH_2, 0xFFFFFFFC);
            test_and_print("testL_c2", testCL_2, 0);


            con_next_cycle(1);

            ull testDL_3 = ((ValRepBase)_md->d0).get_val();
            test_and_print("testDL_3", testDL_3, 1);

            // for (int i = 0; i < 2; i++){
            //     con_end_cycle();
            //     test_and_print("dowhile_pre", ull(_md->x),0);
            //     con_next_cycle(1);
            // }
            // for(int i = 1; i <= 6; i++){
            //     con_end_cycle();
            //     test_and_print("dowhile_val", ull(_md->x),i);
            //     con_next_cycle(1);
            // }
            // for (int i = 0; i < 2; i++){
            //     con_end_cycle();
            //     test_and_print("dowhile_post", ull(_md->x),0);
            //     con_next_cycle(1);
            // }
        }
    };


    class Sim37TestEle: public AutoTestEle{
    public:
        explicit Sim37TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod37, 1);
            start_model_kathryn();
            sim37 simulator((test_sim_mod37*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim37TestEle ele37(37);
}