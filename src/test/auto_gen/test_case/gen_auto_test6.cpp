//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod6: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_reg(b, 8);
        m_reg(c, 8);
        explicit test_gen_mod6(int x): Module(){}

        void flow() override{

            m_wire(plus_reg, 8);
            m_reg (mins_reg, 8);
            m_reg (bw_and_reg, 8);
            m_reg (bw_or_reg, 8);

            par{
                a        <<= hf1;
                b        <<= hf1;
                c        <<= hf1;
                plus_reg  =   hf1 + hf2;
                mins_reg  <<= hf1 - hf2;
                bw_and_reg <<= hf1 & hf2;
                bw_or_reg  <<= hf1 | hf2;
            }

            a       .as_output_glob("a");
            b       .as_output_glob("b");
            c       .as_output_glob("c");
            plus_reg .as_output_glob("plus_reg");
            mins_reg .as_output_glob("mins_reg");
            bw_and_reg.as_output_glob("bw_and_reg");
            bw_or_reg .as_output_glob("bw_or_reg");

        }

    };

    class GenEle6: public GenEle{
    public:
        explicit GenEle6(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod6, 1);
        }
    };

    GenEle6 test_gen6(6);

}