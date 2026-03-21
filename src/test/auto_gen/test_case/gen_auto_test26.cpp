//
// Created by tanawin on 2/12/2025.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod26: public Module{
    public:
        m_val (hf1, 8, 36);
        m_val (hf2, 8, 12);
        m_reg (a, 8);
        m_reg (b, 8);
        m_reg (c, 8);
        m_wire(d, 8);
        ////////test io
        //m_in (e, 8);
        //m_in (f, 8);
        // m_out(g, 8);
        // m_out(h, 8);
        explicit test_gen_mod26(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");

        }

        void flow() override{
            //////// connect output wire
            a.make_reset_event(2);

            zif (a < 16){
                a <<= a + 3;
                a <<= a + 2;
                zif (a < 8){
                    a <<= a + 1;
                }
            }zelif(a < 32){
                zif (a < 24){
                    a <<= 24;
                }zelse{
                    a <<= 48;
                }
            }


        }

    };

    class GenEle26: public GenEle{
    public:
        explicit GenEle26(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod26, 1);
        }
    };

    GenEle26 test_gen26(26);

}