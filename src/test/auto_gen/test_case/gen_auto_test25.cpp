//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{


    class sub_mod25: public Module{
    public: //// io wire
        m_in (i1, 8);m_in (i2, 8);
        m_out(e1, 8);m_out(e2, 8);
        ///////
        m_reg(r1, 8);
        m_reg(r2, 8);

        explicit sub_mod25(int x): Module(){}

        void flow() override{
            e1 = r1;
            e2 = r2;

            seq{
                r1 <<= i1;
                r2 <<= i2;
            }
        }
    };

    class test_gen_mod25: public Module{
    public:
        m_val  (hf1, 8, 36);
        m_val  (hf2, 8, 12);
        m_reg  (a, 8);
        m_reg  (b, 8);
        m_wire (c, 8);
        m_wire (d, 8);
        m_mod  (sm, sub_mod25, 1);

        explicit test_gen_mod25(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");

        }

        void flow() override{
            ////////// we use d as a interrupt
            sm.i1 = a;
            sm.i2 = b;
            c     = sm.e1;
            d     = sm.e2;
            par{
                a = 48;
                b = 49;

            }
        }
    };



    class GenEle25: public GenEle{
    public:
        explicit GenEle25(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod25, 1);
        }
    };

    GenEle25 test_gen25(25);

}