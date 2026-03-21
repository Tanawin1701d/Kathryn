//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod15: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_reg(d, 8);
        m_mem(storage, 48, 8);
        explicit test_gen_mod15(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;
                }
                par{
                    sc_wait(a == 16);
                    cwhile(a < 15){
                        a <<= a + 1;
                    }
                }
                c <<= 1;


            }
        }

    };

    class GenEle15: public GenEle{
    public:
        explicit GenEle15(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod15, 1);
        }
    };

    GenEle15 test_gen15(15);

}