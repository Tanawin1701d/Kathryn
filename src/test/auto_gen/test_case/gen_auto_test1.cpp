//
// Created by tanawin on 25/6/2024.
//

#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod1: public Module{
    public:
        m_reg(a, 8);
        m_reg(b, 8);
        m_wire(c, 8);

        explicit test_gen_mod1(int x): Module(){}

        void flow() override{

            a.as_output_glob("out_a");
            c.as_input_glob("in_c");

            seq{
                cif(c == 0){
                    a <<= b;
                }
            }

        }

    };

    class GenEle1: public GenEle{

    public:
        explicit GenEle1(int id):GenEle(id){}

        void start(PARAM& param) override{
            m_mod(my_md, test_gen_mod1, 1);
        }

    };

    GenEle1 test_case(1);

}
