//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod5: public Module{
    public:
        m_val(con, 8, 48);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        explicit test_gen_mod5(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
        }

        void flow() override{

            seq{
                a <<= con;
                b <<= a;
                c <<= b;
            }
        }

    };

    class GenEle5: public GenEle{
    public:
        explicit GenEle5(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod5, 1);
        }
    };

    GenEle5 test_gen5(5);

}