//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod23: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_wire(d, 8);
        explicit test_gen_mod23(int x): Module(){
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

                a <<= 1;
                cdowhile(c <= 5){
                    par{
                        b = c;
                        c <<= c + 1;
                    }
                }

            }


        }

    };

    class GenEle23: public GenEle{
    public:
        explicit GenEle23(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod23, 1);
        }
    };

    GenEle23 test_gen23(23);

}