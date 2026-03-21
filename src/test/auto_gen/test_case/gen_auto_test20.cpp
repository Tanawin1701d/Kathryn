//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod20: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_wire(d, 1);
        explicit test_gen_mod20(int x): Module(){
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

                par{
                    cwhile(  c == 1 ){
                        cif(a < 10){ intr_start(d); ////// it start at cif not inside cif
                            a = a+1;
                        }celse{
                            a = a+2;
                        };

                        sif(b < 7){ intr_start(d); ////// it start at cif not inside sif
                            b = b+1;
                        }selif(b < 23){
                            b = b+2;
                        }
                    }
                }

            }

            seq{
                sy_wait(5);
                par{ d = 1; c = 1;}
            }


        }

    };

    class GenEle20: public GenEle{
    public:
        explicit GenEle20(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod20, 1);
        }
    };

    GenEle20 test_gen20(20);

}