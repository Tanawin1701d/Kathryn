//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod17: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_reg(d, 8);
        m_mem(storage, 48, 8);
        explicit test_gen_mod17(int x): Module(){
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

                cwhile(a < 47){
                    par{
                        storage[a(0, 6)] <<= a;
                        a <<= a + 1;
                    }
                }

                cwhile(b < 47){
                    par{
                        c <<= storage[b(0,6)];
                        b <<= b + 1;
                    }
                }

            }
        }

    };

    class GenEle17: public GenEle{
    public:
        explicit GenEle17(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod17, 1);
        }
    };

    GenEle17 test_gen17(17);

}