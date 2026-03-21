//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod11: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        m_reg(d, 8);
        explicit test_gen_mod11(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 14;
                    b = 0;
                    c = 0;
                    d = 0;
                }
                par{
                    sif(a < hf2){
                        b <<= b + 1;
                    }selif( a < hf1){
                        cif (d == 0){
                            d <<= d + 1;
                        }celse{
                            d <<= d + 2;
                        }
                    }selse{
                        c <<= c + 1;
                    }

                }
            }
        }

    };

    class GenEle11: public GenEle{
    public:
        explicit GenEle11(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod11, 1);
        }
    };

    GenEle11 test_gen11(11);

}