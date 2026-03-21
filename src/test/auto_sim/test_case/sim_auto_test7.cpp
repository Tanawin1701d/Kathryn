//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod7: public Module{
    public:
        bool test_auto_skip = false;
        m_val(bnk, 32, 48);
        m_val(zero, 32, 0);
        m_val(hf, 32, 64);
        m_val(nine, 32, 9);
        m_val(ele, 32, 11);
        m_reg(a , 32);
        m_reg(b , 32);
        m_reg(c , 32);

        m_reg(s , 32);
        m_reg(s2 , 32);
        m_reg(s3 , 32);

        m_reg(cnt, 2);
        m_val(one, 2, 1);
        m_val(max_cnt, 2, 0);


        explicit test_sim_mod7(bool test_auto_skip): Module(){}

        void flow() override{

            seq{
                a <<= bnk;
                cnt <<= zero;
                b <<= zero;
                par {
                    cif(a > hf) {
                        s <<= nine;
                        s2 <<= nine;
                        s3 <<= nine;
                    }celif(a > nine) {
                        seq {
                            s <<= ele;
                            s2 <<= ele;
                            cwhile(cnt == max_cnt){
                                /** to test sync Reg*/
                                cnt <<= cnt + one;
                            }
                            s3 <<= ele;
                        }
                    }
                }
                c <<= bnk;
            }

        }

    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test7.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test7.vcd";


    class sim7 :public SimAutoInterface{
    public:

        test_sim_mod7* _md;

        sim7(test_sim_mod7* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              100,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                              sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            inc_cycle(6);

            sim {
                ull test_val = 11;
                test_and_print("check middle con", (ull)_md->s, test_val);
            };

        }

        void sim_driven() override{
            inc_cycle(2);

            sim {
                //_md->iv.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
            };
            inc_cycle(1);
        }

    };


    class Sim7TestEle: public AutoTestEle{
    public:
        explicit Sim7TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod7, 1);
            start_model_kathryn();
            sim7 simulator((test_sim_mod7*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

    Sim7TestEle ele7(7);

    ///sim2 test_case2;




}