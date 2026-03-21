//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod7: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        explicit test_gen_mod7(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
        }

        void flow() override{

            seq{
                par{
                    a <<= hf1;
                    b <<= hf2;
                }
                par{
                    a <<= a + hf2;
                    c <<= a + b;
                }
            }

        }

    };

    class GenEle7: public GenEle{
    public:
        explicit GenEle7(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod7, 1);
        }
    };

    GenEle7 test_gen7(7);

}