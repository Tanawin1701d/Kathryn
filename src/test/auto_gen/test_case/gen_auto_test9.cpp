//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod9: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_reg(d, 8);
        explicit test_gen_mod9(int x): Module(){
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
                    cwhile(a < 48){
                        cif( a < 10){
                            b <<= b + 1;
                            a <<= a + 1;
                        }celif( a < 20){
                            c <<= c + 1;
                            a <<= a + 1;
                        }celse{
                            seq{
                                d <<= d + 1;
                                d <<= d + 1;
                                a <<= a + 1;
                            }
                        }
                    }
                }

            }
        }

    };

    class GenEle9: public GenEle{
    public:
        explicit GenEle9(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod9, 1);
        }
    };

    GenEle9 test_gen9(9);

}