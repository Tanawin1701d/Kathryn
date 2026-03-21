//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod21: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_wire(d, 1);
        explicit test_gen_mod21(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");
        }

        void flow() override{

            ////////// we use d as a interrupt
            seq{
                par{
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;
                }

                cwhile(  c == 1 ){
                    cwhile(true){
                        intr_start(d);
                        seq{
                            cif(a == 48){
                                sbreak;
                            }celse{
                                a = a + 1;
                            }
                        }
                    }
                    b = 1;
                }


            }

            seq{
                sy_wait(5);
                d = 1;
            }


        }

    };

    class GenEle21: public GenEle{
    public:
        explicit GenEle21(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod21, 1);
        }
    };

    GenEle21 test_gen21(21);

}