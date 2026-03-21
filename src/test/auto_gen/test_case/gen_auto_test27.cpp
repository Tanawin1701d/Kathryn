//
// Created by tanawin on 2/12/2025.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod27: public Module{
    public:
        m_val (hf1, 8, 36);
        m_val (hf2, 8, 12);
        m_reg (a, 8);
        m_reg (b, 8);
        m_reg (c, 8);
        m_wire(d, 8);
        m_reg(switch_val, 3);
        m_reg(sub_check, 1);
        ////////test io
        //m_in (e, 8);
        //m_in (f, 8);
        // m_out(g, 8);
        // m_out(h, 8);
        explicit test_gen_mod27(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");

        }

        void flow() override{
            //////// connect output wire
            a.make_reset_event(2);

            seq{
                par{
                    switch_val <<= 4;
                    sub_check  <<= 1;
                }

                par{
                    ztate(switch_val){
                        zcase(0b100){
                            a <<=  9;
                            b <<= 24;
                            zif(sub_check){
                                b <<= 48;
                            }
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

    class GenEle27: public GenEle{
    public:
        explicit GenEle27(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod27, 1);
        }
    };

    GenEle27 test_gen27(27);

}