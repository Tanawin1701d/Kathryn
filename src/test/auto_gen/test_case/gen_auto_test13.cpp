//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod13: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_reg(d, 8);
        explicit test_gen_mod13(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 2;
                    b = 4;
                    c = 0;
                    d = 0;
                }
                par{
                    d(2,6) <<= b(2, 6) + a(0, 4);
                }
            }
        }

    };

    class GenEle13: public GenEle{
    public:
        explicit GenEle13(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod13, 1);
        }
    };

    GenEle13 test_gen13(13);

}