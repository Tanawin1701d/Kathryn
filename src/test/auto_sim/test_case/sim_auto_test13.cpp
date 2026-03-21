//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/auto_sim/sim_auto_interface.h"
#include "auto_test_interface.h"

namespace kathryn{

    class test_sim_mod13: public Module{
    public:

        m_reg(a  , 67);
        m_reg(a2  , 67);
        m_reg(a3  , 67);
        m_reg(b  , 33);
        m_reg(b2  ,33);
        m_reg(b3 , 33);
        m_val(BNK,  8, 138);
        m_val(one,  1, 1);
        m_val(five, 67, 5);
        m_val(ss,   8, 66);
        m_val(ten, 67, 10);
        m_wire(x1, 5);
        m_wire(x2, 5);

        explicit test_sim_mod13(int x): Module(){}

        void flow() override{

            seq{
                a2 <<= BNK;
                a2 <<= BNK;
                cwhile(a < a2){
                    a <<= a +  one;

                    cif(a >= ten) {
                        b <<= b + one;
                    }celif(a >= five){
                        b3 <<= b3 + one;
                        x1 = x2;
                        //x2 = x1;
                    }celse{
                        b2 <<= b2 + one;
                    }


                }
            }

        }
    };

    ///static std::string vcd_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/sim_auto_test13.vcd";
    ////static std::string profile_path = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/prof_auto_test13.vcd";


    class sim13 :public SimAutoInterface{
    public:

        test_sim_mod13* _md;

        sim13(test_sim_mod13* md, int idx, const std::string& prefix,
            SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,
                                              200,
                                              prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
                                              prefix + "sim_auto_result"+std::to_string(idx)+".prof",
                                              sim_proxy_build_mode),
                             _md(md)
        {}

        void sim_assert() override{


            inc_cycle(6);
//            sim {
//                ValRep test_val = NumConverter::cvt_str_to_val_rep(67, 0b100, 0);
//                test_and_print("check shifting from slicing", _md->a.sv(), test_val);
//            };
//            sim {
//                ValRep test_val = NumConverter::cvt_str_to_val_rep(8, 2 + 6*2);
//                test_and_print("check base line function", _md->a.sv(), test_val);
//            };
//            sim{
//                ValRep test_val = NumConverter::cvt_str_to_val_rep(8, 48 - 6*2);
//                test_and_print("check bascheck base line functione line function", _md->b.sv(), test_val);
//            };

        }

        void sim_driven() override{
            inc_cycle(2);

//            sim {
//                _md->a.sv() = NumConverter::cvt_str_to_val_rep(8,  2);
//                _md->b.sv() = NumConverter::cvt_str_to_val_rep(8, 48);
//            };

        }

    };


    class Sim13TestEle: public AutoTestEle{
    public:
        explicit Sim13TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
            m_mod(d, test_sim_mod13, 1);
            start_model_kathryn();
            sim13 simulator((test_sim_mod13*) &d, _simId, prefix, sim_proxy_build_mode);
            simulator.sim_start();
        }

    };

#ifndef NOTEXCEED64
    Sim13TestEle ele13(13);
#endif

}