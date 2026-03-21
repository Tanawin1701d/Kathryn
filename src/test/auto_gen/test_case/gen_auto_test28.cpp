//
// Created by tanawin on 6/1/2026.
//
#include "kathryn.h"
#include "gen_ele.h"
#include "lib/math/math.h"

namespace kathryn{

    class test_gen_mod28: public Module{
    public:
        m_reg (a, 8);
        m_reg (b, 8);
        m_reg (c, 8);
        m_wire(d, 8);
        explicit test_gen_mod28(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");
        }

        void flow() override{
            //////// connect output wire
            seq{
                a <<= 40;
                b <<= sqrt_int(a);
            }
        }

    };

    class GenEle28: public GenEle{
    public:
        explicit GenEle28(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod28, 1);
        }
    };

    GenEle28 test_gen28(28);

}
